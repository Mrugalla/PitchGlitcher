#pragma once
#include <array>
#include "Using.h"

namespace gui
{
    enum class ColourID
    {
        Bg,
        Txt,
        Abort,
        Interact,
        Inactive,
        Darken,
        Hover,
        Transp,
        Mod,
        Bias,
        NumCols
    };

    inline Colour getDefault(ColourID i) noexcept
    {
        switch (i)
        {
        case ColourID::Bg: return Colour(0xff181620);
        case ColourID::Txt: return Colour(0xff009aff);
        case ColourID::Inactive: return Colour(0xff808080);
        case ColourID::Abort: return Colour(0xffff0000);
        case ColourID::Interact: return Colour(0xff00bcab);
        case ColourID::Darken: return Colour(0xea000000);
        case ColourID::Hover: return Colour(0x756e89bd);
        case ColourID::Mod: return Colour(0xffd20082);
        case ColourID::Bias: return Colour(0xffcaaa00);
        default: return Colour(0x00000000);
        }
    }
    inline String toString(ColourID i)
    {
        switch (i)
        {
        case ColourID::Bg: return "background";
        case ColourID::Txt: return "text";
        case ColourID::Abort: return "abort";
        case ColourID::Interact: return "interact";
        case ColourID::Inactive: return "inactive";
        case ColourID::Darken: return "darken";
        case ColourID::Hover: return "hover";
        case ColourID::Transp: return "transp";
        case ColourID::Mod: return "mod";
        case ColourID::Bias: return "bias";
        default: return "";
        }
    }

    inline String toStringProps(ColourID i)
    {
        return "colour" + toString(i);
    }

    class Colours
    {
        using Array = std::array<Colour, static_cast<int>(ColourID::NumCols)>;
    public:
        Colours() :
            cols(),
            props(nullptr)
        {}

        void init(Props* p)
        {
            props = p;
            if (props->isValidFile())
                for (auto i = 0; i < static_cast<int>(ColourID::NumCols); ++i)
                {
                    const auto cID = static_cast<ColourID>(i);
                    const auto colStr = props->getValue(toStringProps(cID), getDefault(cID).toString());
                    set(i, juce::Colour::fromString(colStr));
                }
        }

        bool set(const String& i, Colour col)
        {
            for (auto j = 0; j < cols.size(); ++j)
                if (i == cols[j].toString())
                    return set(j, col);
            return false;
        }

        bool set(ColourID i, Colour col) noexcept
        {
            return set(static_cast<int>(i), col);
        }

        bool set(int i, Colour col) noexcept
        {
            if (props->isValidFile())
            {
                cols[i] = col;
                props->setValue(toStringProps(ColourID(i)), col.toString());
                if (props->needsToBeSaved())
                {
                    props->save();
                    props->sendChangeMessage();
                    return true;
                }
            }
            return false;
        }

        Colour operator()(ColourID i) const noexcept
        {
            return get(static_cast<int>(i));
        }

        Colour operator()(int i) const noexcept
        {
            return get(i);
        }

        Colour get(int i) const noexcept
        {
            return cols[i];
        }

        static Colours c;
    protected:
        Array cols;
        Props* props;
    };

    // GET FONT
    inline Font getFont(const char* ttf, size_t size)
    {
        return juce::Font(juce::Typeface::createSystemTypefaceFor(ttf, size));
    }
    
    // GET FONT NEL
    inline Font getFontNEL()
    {
        return getFont(BinaryData::nel19_ttf, BinaryData::nel19_ttfSize);
    }

    // GET FONT LOBSTER
    inline Font getFontLobster()
    {
        return getFont(BinaryData::LobsterRegular_ttf, BinaryData::LobsterRegular_ttfSize);
    }

    // GET FONT MS MADI
    inline Font getFontMsMadi()
    {
        return getFont(BinaryData::MsMadiRegular_ttf, BinaryData::MsMadiRegular_ttfSize);
    }

    // GET FONT DOSIS
    inline Font getFontDosisSemiBold()
    {
        return getFont(BinaryData::DosisSemiBold_ttf, BinaryData::DosisSemiBold_ttfSize);
    }

    inline Font getFontDosisBold()
    {
        return getFont(BinaryData::DosisBold_ttf, BinaryData::DosisBold_ttfSize);
    }

    inline Font getFontDosisExtraBold()
    {
        return getFont(BinaryData::DosisExtraBold_ttf, BinaryData::DosisExtraBold_ttfSize);
    }

    inline Font getFontDosisLight()
    {
        return getFont(BinaryData::DosisLight_ttf, BinaryData::DosisLight_ttfSize);
    }

    inline Font getFontDosisExtraLight()
    {
        return getFont(BinaryData::DosisExtraLight_ttf, BinaryData::DosisExtraLight_ttfSize);
    }

    inline Font getFontDosisMedium()
    {
        return getFont(BinaryData::DosisMedium_ttf, BinaryData::DosisMedium_ttfSize);
    }

    inline Font getFontDosisRegular()
    {
        return getFont(BinaryData::DosisRegular_ttf, BinaryData::DosisRegular_ttfSize);
    }

    inline Font getFontDosisVariable()
    {
        return getFont(BinaryData::DosisVariableFont_wght_ttf, BinaryData::DosisVariableFont_wght_ttfSize);
    }
}