#include "Param.h"
#include "../config.h"

param::String param::toID(const String& name)
{
	return name.removeCharacters(" ").toLowerCase();
}

param::PID param::ll(PID pID, int offset) noexcept
{
	auto i = static_cast<int>(pID);
	i += (NumLowLevelParams - 1) * offset;
	return static_cast<PID>(i);
}

param::String param::toString(PID pID)
{
	switch (pID)
	{
	case PID::Macro: return "Macro";
#if PPDHasGainIn
	case PID::GainIn: return "Gain In";
#endif
	case PID::Mix: return "Mix";
	case PID::Gain: return "Gain Out";
#if PPDHasHQ
	case PID::HQ: return "HQ";
#endif
#if PPDHasPolarity
	case PID::Polarity: return "Polarity";
#endif
#if PPDHasUnityGain
	case PID::UnityGain: return "Unity Gain";
#endif
#if PPDHasStereoConfig
	case PID::StereoConfig: return "Stereo Config";
#endif

	case PID::Power: return "Power";

		// LOW LEVEL PARAMS:
	case PID::TuneSemi: return "Tune Semi";
	case PID::TuneFine: return "Tune Fine";
	case PID::GrainSize: return "Grain Size";
	case PID::Feedback: return "Feedback";
	case PID::NumVoices: return "Num Voices";
	case PID::SpreadTune: return "Spread Tune";
	
	default: return "Invalid Parameter Name";
	}
}

param::PID param::toPID(const String& id)
{
	for (auto i = 0; i < NumParams; ++i)
	{
		auto pID = static_cast<PID>(i);
		if (id == toID(toString(pID)))
			return pID;
	}
	return PID::NumParams;
}

param::String param::toTooltip(PID pID)
{
	switch (pID)
	{
	case PID::Macro: return "Dial in the desired amount of macro modulation depth.";
#if PPDHasGainIn
	case PID::GainIn: return "Apply input gain to the wet signal.";
#endif
	case PID::Mix: return "Mix the dry with the wet signal.";
	case PID::Gain: return "Apply output gain to the wet signal.";
#if PPDHasHQ
	case PID::HQ: return "Turn on HQ to apply 2x Oversampling to the signal.";
#endif
#if PPDHasPolarity
	case PID::Polarity: return "Invert the wet signal's polarity.";
#endif
#if PPDHasUnityGain
	case PID::UnityGain: return "If enabled the inversed input gain gets added to the output gain.";
#endif
#if PPDHasStereoConfig
	case PID::StereoConfig: return "Define the stereo-configuration. L/R or M/S.";
#endif

	case PID::Power: return "Bypass the plugin with this parameter.";

	case PID::TuneSemi: return "You can retune your signal with this parameter in semitones.";
	case PID::TuneFine: return "You can retune your signal with this parameter in finetones.";
	case PID::GrainSize: return "Use this to define the texture of the pitchshifter.";
	case PID::Feedback: return "This pitchshifter appears to possess a perculiar kind of feedback.";
	case PID::NumVoices: return "The number of parallel voices used to pitchshift.";
	case PID::SpreadTune: return "How spread out the tune values of the voices are.";
	
	default: return "Invalid Tooltip.";
	}
}

param::String param::toString(Unit pID)
{
	switch (pID)
	{
	case Unit::Power: return "";
	case Unit::Solo: return "S";
	case Unit::Mute: return "M";
	case Unit::Percent: return "%";
	case Unit::Hz: return "hz";
	case Unit::Beats: return "x";
	case Unit::Degree: return CharPtr("\xc2\xb0");
	case Unit::Octaves: return "oct";
	case Unit::Semi: return "semi";
	case Unit::Fine: return "fine";
	case Unit::Ms: return "ms";
	case Unit::Decibel: return "db";
	case Unit::Ratio: return "ratio";
	case Unit::Polarity: return CharPtr("\xc2\xb0");
	case Unit::StereoConfig: return "";
	case Unit::Voices: return "v";
	default: return "";
	}
}

param::Param::Param(const PID pID, const Range& _range, const float _valDenormDefault,
	const ValToStrFunc& _valToStr, const StrToValFunc& _strToVal,
	State& _state, const Unit _unit, bool _locked) :

	AudioProcessorParameter(),
	id(pID),
	range(_range),

	state(_state),
	valDenormDefault(_valDenormDefault),

	valNorm(range.convertTo0to1(_valDenormDefault)),
	maxModDepth(0.f),
	valMod(valNorm.load()),
	modBias(.5f),

	valToStr(_valToStr),
	strToVal(_strToVal),
	unit(_unit),

	locked(_locked),
	inGesture(false)
{
}

void param::Param::savePatch(juce::ApplicationProperties& appProps) const
{
	const auto v = range.convertFrom0to1(getValue());
	state.set(getIDString(), "value", v, true);
	const auto mdd = getMaxModDepth();
	state.set(getIDString(), "maxmoddepth", mdd, true);
	const auto mb = getModBias();
	state.set(getIDString(), "modbias", mb, true);

	auto user = appProps.getUserSettings();
	if (user->isValidFile())
	{
		user->setValue(getIDString() + "valDefault", valDenormDefault);
	}
}

void param::Param::loadPatch(juce::ApplicationProperties& appProps)
{
	const auto lckd = isLocked();
	if (!lckd)
	{
		auto var = state.get(getIDString(), "value");
		if (var)
		{
			const auto val = static_cast<float>(*var);
			const auto valD = range.convertTo0to1(val);
			setValueNotifyingHost(valD);
		}
		var = state.get(getIDString(), "maxmoddepth");
		if (var)
		{
			const auto val = static_cast<float>(*var);
			setMaxModDepth(val);
		}
		var = state.get(getIDString(), "modbias");
		if (var)
		{
			const auto val = static_cast<float>(*var);
			setModBias(val);
		}
		auto user = appProps.getUserSettings();
		if (user->isValidFile())
		{
			const auto vdd = user->getDoubleValue(getIDString() + "valDefault", static_cast<double>(valDenormDefault));
			setDefaultValue(range.convertTo0to1(static_cast<float>(vdd)));
		}
	}
}

//called by host, normalized, thread-safe
float param::Param::getValue() const { return valNorm.load(); }
float param::Param::getValueDenorm() const noexcept { return range.convertFrom0to1(getValue()); }

// called by host, normalized, avoid locks, not used by editor
void param::Param::setValue(float normalized)
{
	if (!isLocked())
		valNorm.store(normalized);
}

// called by editor
bool param::Param::isInGesture() const noexcept
{
	return inGesture.load();
}

void param::Param::setValueWithGesture(float norm)
{
	if (isInGesture())
		return;
	beginChangeGesture();
	setValueNotifyingHost(norm);
	endChangeGesture();
}

void param::Param::beginGesture()
{
	inGesture.store(true);
	beginChangeGesture();
}

void param::Param::endGesture()
{
	inGesture.store(false);
	endChangeGesture();
}

float param::Param::getMaxModDepth() const noexcept { return maxModDepth.load(); };

void param::Param::setMaxModDepth(float v) noexcept
{
	if (isLocked())
		return;

	maxModDepth.store(juce::jlimit(-1.f, 1.f, v));
}

float param::Param::getValMod() const noexcept { return valMod.load(); }

float param::Param::getValModDenorm() const noexcept { return range.convertFrom0to1(valMod.load()); }

void param::Param::setModBias(float b) noexcept
{
	if (isLocked())
		return;

	b = juce::jlimit(BiasEps, 1.f - BiasEps, b);
	modBias.store(b);
}

float param::Param::getModBias() const noexcept { return modBias.load(); }

void param::Param::setDefaultValue(float norm) noexcept
{
	valDenormDefault = range.convertFrom0to1(norm);
}

// called by processor to update modulation value(s)
void param::Param::modulate(float macro) noexcept
{
	const auto norm = getValue();

	const auto mmd = maxModDepth.load();
	const auto pol = mmd > 0.f ? 1.f : -1.f;
	const auto md = mmd * pol;
	const auto mdSkew = biased(0.f, md, modBias.load(), macro);
	const auto mod = mdSkew * pol;

	valMod.store(juce::jlimit(0.f, 1.f, norm + mod));
}

float param::Param::getDefaultValue() const { return range.convertTo0to1(valDenormDefault); }

param::String param::Param::getName(int) const { return toString(id); }

// units of param (hz, % etc.)
param::String param::Param::getLabel() const { return toString(unit); }

// string of norm val
param::String param::Param::getText(float norm, int) const
{
	return valToStr(range.snapToLegalValue(range.convertFrom0to1(norm)));
}

// string to norm val
float param::Param::getValueForText(const String& text) const
{
	const auto val = juce::jlimit(range.start, range.end, strToVal(text));
	return range.convertTo0to1(val);
}

	// string to denorm val
float param::Param::getValForTextDenorm(const String& text) const { return strToVal(text); }

param::String param::Param::_toString()
{
	auto v = getValue();
	return getName(10) + ": " + String(v) + "; " + getText(v, 10);
}

bool param::Param::isLocked() const noexcept { return locked.load(); }

void param::Param::setLocked(bool e) noexcept { locked.store(e); }

void param::Param::switchLock() noexcept { setLocked(!isLocked()); }

param::String param::Param::getIDString() const
{
	return "params/" + toID(toString(id));
}

float param::Param::biased(float start, float end, float bias/*[0,1]*/, float x) const noexcept
{
	const auto r = end - start;
	if (r == 0.f)
		return 0.f;
	const auto a2 = 2.f * bias;
	const auto aM = 1.f - bias;
	const auto aR = r * bias;
	return start + aR * x / (aM - x + a2 * x);
}

std::function<float(param::String, const float/*altVal*/)> param::strToVal::parse()
{
	return [](const String& txt, const float altVal)
	{
		parser::Parser parse;
		if (parse(txt))
			return parse[0];

		return altVal;
	};
}

param::StrToValFunc param::strToVal::power()
{
	return [p = parse()](const String& txt)
	{
		const auto text = txt.trimCharactersAtEnd(toString(Unit::Power));
		const auto val = p(text, 0.f);
		return val > .5f ? 1.f : 0.f;
	};
}

param::StrToValFunc param::strToVal::solo()
{
	return [p = parse()](const String& txt)
	{
		const auto text = txt.trimCharactersAtEnd(toString(Unit::Solo));
		const auto val = p(text, 0.f);
		return val > .5f ? 1.f : 0.f;
	};
}

param::StrToValFunc param::strToVal::mute()
{
	return [p = parse()](const String& txt)
	{
		const auto text = txt.trimCharactersAtEnd(toString(Unit::Mute));
		const auto val = p(text, 0.f);
		return val > .5f ? 1.f : 0.f;
	};
}

param::StrToValFunc param::strToVal::percent()
{
	return [p = parse()](const String& txt)
	{
		const auto text = txt.trimCharactersAtEnd(toString(Unit::Percent));
		const auto val = p(text, 0.f);
		return val * .01f;
	};
}

param::StrToValFunc param::strToVal::hz()
{
	return [p = parse()](const String& txt)
	{
		const auto text = txt.trimCharactersAtEnd(toString(Unit::Hz));
		const auto val = p(text, 0.f);
		return val;
	};
}

param::StrToValFunc param::strToVal::phase()
{
	return [p = parse()](const String& txt)
	{
		const auto text = txt.trimCharactersAtEnd(toString(Unit::Degree));
		const auto val = p(text, 0.f);
		return val;
	};
}

param::StrToValFunc param::strToVal::oct()
{
	return [p = parse()](const String& txt)
	{
		const auto text = txt.trimCharactersAtEnd(toString(Unit::Octaves));
		const auto val = p(text, 0.f);
		return std::floor(val);
	};
}

param::StrToValFunc param::strToVal::oct2()
{
	return [p = parse()](const String& txt)
	{
		const auto text = txt.trimCharactersAtEnd(toString(Unit::Octaves));
		const auto val = p(text, 0.f);
		return val / 12.f;
	};
}

param::StrToValFunc param::strToVal::semi()
{
	return [p = parse()](const String& txt)
	{
		const auto text = txt.trimCharactersAtEnd(toString(Unit::Semi));
		const auto val = p(text, 0.f);
		return std::floor(val);
	};
}

param::StrToValFunc param::strToVal::fine()
{
	return [p = parse()] (const String& txt)
	{
		const auto text = txt.trimCharactersAtEnd(toString(Unit::Fine));
		const auto val = p(text, 0.f);
		return val * .01f;
	};
}

param::StrToValFunc param::strToVal::ratio()
{
	return [p = parse()](const String& txt)
	{
		const auto text = txt.trimCharactersAtEnd(toString(Unit::Ratio));
		const auto val = p(text, 0.f);
		return val * .01f;
	};
}

param::StrToValFunc param::strToVal::lrms()
{
	return [](const String& txt)
	{
		return txt[0] == 'l' ? 0.f : 1.f;
	};
}

param::StrToValFunc param::strToVal::freeSync()
{
	return [](const String& txt)
	{
		return txt[0] == 'f' ? 0.f : 1.f;
	};
}

param::StrToValFunc param::strToVal::polarity()
{
	return [](const String& txt)
	{
		return txt[0] == '0' ? 0.f : 1.f;
	};
}

param::StrToValFunc param::strToVal::ms()
{
	return[p = parse()](const String& txt)
	{
		const auto text = txt.trimCharactersAtEnd(toString(Unit::Ms));
		const auto val = p(text, 0.f);
		return val;
	};
}

param::StrToValFunc param::strToVal::db()
{
	return[p = parse()](const String& txt)
	{
		const auto text = txt.trimCharactersAtEnd(toString(Unit::Decibel));
		const auto val = p(text, 0.f);
		return val;
	};
}

param::StrToValFunc param::strToVal::voices()
{
	return[p = parse()](const String& txt)
	{
		const auto text = txt.trimCharactersAtEnd(toString(Unit::Voices));
		const auto val = p(text, 1.f);
		return val;
	};
}


param::ValToStrFunc param::valToStr::mute()
{
	return [](float v) { return v > .5f ? "Mute" : "Not Mute"; };
}

param::ValToStrFunc param::valToStr::solo()
{
	return [](float v) { return v > .5f ? "Solo" : "Not Solo"; };
}

param::ValToStrFunc param::valToStr::power()
{
	return [](float v) { return v > .5f ? "Enabled" : "Disabled"; };
}

param::ValToStrFunc param::valToStr::percent()
{
	return [](float v) { return String(std::floor(v * 100.f)) + " " + toString(Unit::Percent); };
}

param::ValToStrFunc param::valToStr::hz()
{
	return [](float v)
	{
		if(v >= 10000.f)
			return String(v).substring(0, 5) + " " + toString(Unit::Hz);
		else if(v >= 1000.f)
			return String(v).substring(0, 4) + " " + toString(Unit::Hz);
		else
			return String(v).substring(0, 5) + " " + toString(Unit::Hz);
	};
}

param::ValToStrFunc param::valToStr::phase()
{
	return [](float v) { return String(std::floor(v * 180.f)) + " " + toString(Unit::Degree); };
}

param::ValToStrFunc param::valToStr::phase360()
{
	return [](float v) { return String(std::floor(v * 360.f)) + " " + toString(Unit::Degree); };
}

param::ValToStrFunc param::valToStr::oct()
{
	return [](float v) { return String(std::floor(v)) + " " + toString(Unit::Octaves); };
}

param::ValToStrFunc param::valToStr::oct2()
{
	return [](float v) { return String(std::floor(v / 12.f)) + " " + toString(Unit::Octaves); };
}

param::ValToStrFunc param::valToStr::semi()
{
	return [](float v) { return String(std::floor(v)) + " " + toString(Unit::Semi); };
}

param::ValToStrFunc param::valToStr::fine()
{
	return [](float v) { return String(std::floor(v * 100.f)) + " " + toString(Unit::Fine); };
}

param::ValToStrFunc param::valToStr::ratio()
{
	return [](float v)
	{
		const auto y = static_cast<int>(std::floor(v * 100.f));
		return String(100 - y) + " : " + String(y);
	};
}

param::ValToStrFunc param::valToStr::lrms()
{
	return [](float v) { return v > .5f ? String("m/s") : String("l/r"); };
}

param::ValToStrFunc param::valToStr::freeSync()
{
	return [](float v) { return v > .5f ? String("sync") : String("free"); };
}

param::ValToStrFunc param::valToStr::polarity()
{
	return [](float v) { return v > .5f ? String("on") : String("off"); };
}

param::ValToStrFunc param::valToStr::ms()
{
	return [](float v) { return String(std::floor(v * 10.f) * .1f) + " " + toString(Unit::Ms); };
}

param::ValToStrFunc param::valToStr::db()
{
	return [](float v) { return String(std::floor(v * 100.f) * .01f) + " " + toString(Unit::Decibel); };
}

param::ValToStrFunc param::valToStr::empty()
{
	return [](float) { return String(""); };
}

param::ValToStrFunc param::valToStr::voices()
{
	return [](float v)
	{
		return String(static_cast<int>(v)) + toString(Unit::Voices);
	};
}


param::Param* param::makeParam(PID id, State& state,
	float valDenormDefault, const Range& range,
	Unit unit, bool isLocked)
{
	ValToStrFunc valToStrFunc;
	StrToValFunc strToValFunc;

	switch (unit)
	{
	case Unit::Power:
		valToStrFunc = valToStr::power();
		strToValFunc = strToVal::power();
		break;
	case Unit::Solo:
		valToStrFunc = valToStr::solo();
		strToValFunc = strToVal::solo();
		break;
	case Unit::Mute:
		valToStrFunc = valToStr::mute();
		strToValFunc = strToVal::mute();
		break;
	case Unit::Decibel:
		valToStrFunc = valToStr::db();
		strToValFunc = strToVal::db();
		break;
	case Unit::Ms:
		valToStrFunc = valToStr::ms();
		strToValFunc = strToVal::ms();
		break;
	case Unit::Percent:
		valToStrFunc = valToStr::percent();
		strToValFunc = strToVal::percent();
		break;
	case Unit::Hz:
		valToStrFunc = valToStr::hz();
		strToValFunc = strToVal::hz();
		break;
	case Unit::Ratio:
		valToStrFunc = valToStr::ratio();
		strToValFunc = strToVal::ratio();
		break;
	case Unit::Polarity:
		valToStrFunc = valToStr::polarity();
		strToValFunc = strToVal::polarity();
		break;
	case Unit::StereoConfig:
		valToStrFunc = valToStr::lrms();
		strToValFunc = strToVal::lrms();
		break;
	case Unit::Semi:
		valToStrFunc = valToStr::semi();
		strToValFunc = strToVal::semi();
		break;
	case Unit::Fine:
		valToStrFunc = valToStr::fine();
		strToValFunc = strToVal::fine();
		break;
	case Unit::Voices:
		valToStrFunc = valToStr::voices();
		strToValFunc = strToVal::voices();
		break;
	default:
		valToStrFunc = valToStr::empty();
		strToValFunc = strToVal::percent();
		break;
	}

	return new Param(id, range, valDenormDefault, valToStrFunc, strToValFunc, state, unit, isLocked);
}


param::Params::Params(AudioProcessor& audioProcessor, State& state) :
	params()
{
	params.push_back(makeParam(PID::Macro, state, 0.f));
#if PPDHasGainIn
	params.push_back(makeParam(PID::GainIn, state, 0.f, makeRange::withCentre(PPD_GainIn_Min, PPD_GainIn_Max, 0.f), Unit::Decibel));
#endif
	params.push_back(makeParam(PID::Mix, state));
	params.push_back(makeParam(PID::Gain, state, 0.f, makeRange::withCentre(PPD_GainOut_Min, PPD_GainIn_Max, 0.f), Unit::Decibel));
#if PPDHasPolarity
	params.push_back(makeParam(PID::Polarity, state, 0.f, makeRange::toggle(), Unit::Polarity));
#endif
#if PPDHasUnityGain
	params.push_back(makeParam(PID::UnityGain, state, (PPD_UnityGainDefault ? 1.f : 0.f), makeRange::toggle(), Unit::Polarity));
#endif
#if PPDHasHQ
	params.push_back(makeParam(PID::HQ, state, 1.f, makeRange::toggle()));
#endif
#if PPDHasStereoConfig
	params.push_back(makeParam(PID::StereoConfig, state, 1.f, makeRange::toggle(), Unit::StereoConfig));
#endif
	params.push_back(makeParam(PID::Power, state, 1.f, makeRange::toggle(), Unit::Power));

	// LOW LEVEL PARAMS:
	params.push_back(makeParam(PID::GrainSize, state, 120.f, makeRange::withCentre(10.f, PPDPitchShifterSizeMs, 70.f, .001f), Unit::Ms));
	params.push_back(makeParam(PID::TuneSemi, state, 12.f, makeRange::stepped(-24.f, 24.f, 1.f), Unit::Semi));
	params.push_back(makeParam(PID::TuneFine, state, 0.f, Range(-1.f, 1.f), Unit::Fine));
	params.push_back(makeParam(PID::Feedback, state, .16f));
	static constexpr float NumVoices = static_cast<float>(PPDPitchShifterNumVoices);
	params.push_back(makeParam(PID::NumVoices, state, NumVoices, makeRange::stepped(1.f, NumVoices, 1.f), Unit::Voices));
	params.push_back(makeParam(PID::SpreadTune, state, .19f, makeRange::withCentre(0.f, 1.f, .19f, .001f), Unit::Fine));
	
	// LOW LEVEL PARAMS END

	for (auto param : params)
		audioProcessor.addParameter(param);
}

void param::Params::loadPatch(juce::ApplicationProperties& appProps)
{
	for (auto param : params)
		param->loadPatch(appProps);
}

void param::Params::savePatch(juce::ApplicationProperties& appProps) const
{
	for (auto param : params)
		param->savePatch(appProps);
}

int param::Params::getParamIdx(const String& nameOrID) const
{
	for (auto p = 0; p < params.size(); ++p)
	{
		const auto pName = toString(params[p]->id);
		if (nameOrID == pName || nameOrID == toID(pName))
			return p;
	}
	return -1;
}

size_t param::Params::numParams() const noexcept { return params.size(); }

param::Param* param::Params::operator[](int i) noexcept { return params[i]; }
const param::Param* param::Params::operator[](int i) const noexcept { return params[i]; }
param::Param* param::Params::operator[](PID p) noexcept { return params[static_cast<int>(p)]; }
const param::Param* param::Params::operator[](PID p) const noexcept { return params[static_cast<int>(p)]; }

param::Params::Parameters& param::Params::data() noexcept { return params; }
const param::Params::Parameters& param::Params::data() const noexcept { return params; }

param::MacroProcessor::MacroProcessor(Params& _params) :
	params(_params)
{
}

void param::MacroProcessor::operator()() noexcept
{
	const auto modDepth = params[PID::Macro]->getValue();
	for (auto i = 1; i < NumParams; ++i)
		params[i]->modulate(modDepth);
}

#include "../configEnd.h"