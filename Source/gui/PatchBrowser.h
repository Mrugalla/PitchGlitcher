#pragma once
#include "Button.h"

namespace gui
{
	using ValueTree = juce::ValueTree;
	using Identifier = juce::Identifier;

	struct Patch :
		public Button
	{
		using SharedPatch = std::shared_ptr<Patch>;
		using WeakPatch = std::weak_ptr<Patch>;
		using Patches = std::vector<SharedPatch>;

		Patch(Utils& u, const String& _name, const String& _author) :
			Button(u, "Click here to select and load this patch."),
			name(u, _name),
			author(u, "by " + _author),
			tags(),
			state(utils.getState())
		{
			layout.init(
				{ 1, 8, 8, 1 },
				{ 1 }
			);

			makeSymbolButton(*this, ButtonSymbol::Empty);

			name.font = getFontDosisMedium();
			author.font = name.font;

			name.textCID = ColourID::Txt;
			author.textCID = ColourID::Hover;

			name.just = Just::centredLeft;
			author.just = Just::centredLeft;

			name.mode = Label::Mode::TextToLabelBounds;
			author.mode = Label::Mode::TextToLabelBounds;

			addAndMakeVisible(name);
			addAndMakeVisible(author);
		}

		bool tagExists(const String& str) const noexcept
		{
			for (const auto& t : tags)
				if (t == str)
					return true;
			return false;
		}

		bool addTag(const String& str)
		{
			const auto s = str.toLowerCase();

			if (tagExists(s))
				return false;

			tags.push_back(s);

			// inform tagSelector

			return true;
		}

		bool isSame(const String& _name, const String& _author) const
		{
			return name.getText() == _name &&
				author.getText() == _author;
		}

		bool isSame(Patch& other) const
		{
			return isSame(other.name.getText(), other.author.getText());
		}

		bool isRemovable() const
		{
			return !author.getText().contains("factory");
		}

		void paint(Graphics&) override {}

		void resized() override
		{
			layout.resized();

			layout.place(name, 1, 0, 1, 1, false);
			layout.place(author, 2, 0, 1, 1, false);
		}

		Label name, author;
		std::vector<String> tags;
		ValueTree state;
	};

	String toString(const Patch& patch)
	{
		String str("name: " + patch.name.getText() +
			"\nauthor: " + patch.author.getText() +
			"\ntags: ");
		//for (const auto& tag : patch.tags)
		//	str += String(tag.toString()) + "; ";

		return str;
	}

	using SharedPatch = Patch::SharedPatch;
	using WeakPatch = Patch::WeakPatch;
	using Patches = Patch::Patches;

	struct PatchList :
		public CompScrollable
	{
		static constexpr float RelHeight = 10.f;
		using SortFunc = std::function<bool(const SharedPatch& a, const SharedPatch& b)>;

		PatchList(Utils& u) :
			CompScrollable(u),
			patches(),
			filterString(""),
			selected(nullptr),
			listBounds()
		{
			layout.init(
				{ 21, 1 },
				{ 1 }
			);

			Random rand;
			for (auto i = 0; i < 20; ++i)
			{
				String strA, strB;
				appendRandomString(strA, rand, 12);
				appendRandomString(strB, rand, 12);
				save(strA, strB);
			}
		}

		bool save(const String& _name, const String& _author)
		{
			// check if patch already exists (accept overwrite?)
			for (const auto& patch : patches)
				if (patch->isSame(_name, _author))
					return false;

			patches.push_back(std::make_shared<Patch>(
				utils,
				_name,
				_author
			));

			selected = patches.back();

			selected->onClick.push_back([&, thisPatch = std::weak_ptr<Patch>(selected)]()
			{
				auto locked = thisPatch.lock();

				if (locked == nullptr)
					return;

				selected = locked;
				updateShown();
			});

			addAndMakeVisible(*selected);
			updateShown();

			return true;
		}

		void removeSelected()
		{
			if (selected == nullptr)
				return;

			for (auto i = 0; i < patches.size(); ++i)
			{
				auto ptch = patches[i];
				if (ptch == selected)
				{
					if (ptch->isRemovable())
					{
						removeChildComponent(ptch.get());
						patches.erase(patches.begin() + i);
						selected.reset();
						updateShown();
						return;
					}
				}
			}
		}

		SharedPatch getSelectedPatch() const noexcept
		{
			if (selected == nullptr || patches.empty())
				return nullptr;
			return selected;
		}

		void show(const String& containedString)
		{
			if (filterString == containedString)
				return;

			filterString = containedString;

			updateShown();
		}

		void sort(const SortFunc& sortFunc)
		{
			std::sort(patches.begin(), patches.end(), sortFunc);

			updateShown();
		}

		size_t numPatches() const noexcept { return patches.size(); }

		const SharedPatch operator[](int i) const noexcept { return patches[i]; }
	protected:
		Patches patches;
		String filterString;
		SharedPatch selected;
		BoundsF listBounds;

		void resized() override
		{
			layout.resized();

			layout.place(scrollBar, 1, 0, 1, 1);

			listBounds = layout(0, 0, 1, 1);

			const auto x = listBounds.getX();
			const auto w = listBounds.getWidth();
			const auto h = utils.thicc * RelHeight;
			actualHeight = h * static_cast<float>(patches.size());

			auto y = listBounds.getY() - yScrollOffset;

			for (auto p = 0; p < patches.size(); ++p)
			{
				auto& patch = patches[p];

				if (patch->isVisible())
				{
					patch->setBounds(BoundsF(x, y, w, h).toNearestInt());
					y += h;
				}
			}
		}

		void paint(Graphics& g) override
		{
			if (patches.empty())
			{
				g.setColour(Colours::c(ColourID::Abort));
				g.setFont(getFontLobster().withHeight(24.f));
				g.drawFittedText(
					"sry, this browser does not contain patches yet...",
					getLocalBounds(),
					Just::centred,
					1
				);
				return;
			}

			paintList(g);
		}

		void paintList(Graphics& g)
		{
			auto x = listBounds.getX();
			auto y = listBounds.getY() - yScrollOffset;
			auto w = listBounds.getWidth();
			auto btm = listBounds.getBottom();
			auto r = utils.thicc * RelHeight;

			g.setColour(Colours::c(ColourID::Txt).withAlpha(.1f));
			for (auto i = 0; i < patches.size(); ++i)
			{
				if (y >= btm)
					return;
				if (i % 2 == 0)
					g.fillRect(x, y, w, r);

				const auto ptch = patches[i];

				if (ptch->isVisible() && y >= 0.f)
				{
					if (selected == ptch)
					{
						g.setColour(Colours::c(ColourID::Interact));
						g.drawRect(x, y, w, r);
						g.setColour(Colours::c(ColourID::Txt).withAlpha(.1f));
					}
				}
				y += r;
			}	
		}

		void updateShown()
		{
			//bool considerTags = tags.size() != 0;
			bool considerString = filterString.isNotEmpty();

			if(/*!considerTags && */!considerString)
				for (auto& patch : patches)
					patch->setVisible(true);
			else
			{
				for (auto& patch : patches)
				{
					patch->setVisible(false);
				}

				if(considerString)
					for (auto& patch : patches)
					{
						const auto& patchName = patch->name.getText().toLowerCase();
						if (patchName.contains(filterString))
							patch->setVisible(true);
					}

				/*
				if(considerTags)
					for (auto& patch : patches)
					{
						for (const auto& tag : tags)
						{
							if (patch->has(tag))
								patch->setVisible(true);
						}
					}
				*/
			}

			resized();
			repaintWithChildren(getParentComponent());
		}
	};

	struct PatchListSortable :
		public Comp
	{
		PatchListSortable(Utils& u) :
			Comp(u, "", CursorType::Default),
			patchList(u),
			sortByName(u, "click here to sort patches by name."),
			sortByAuthor(u, "click here to sort patches by author.")
		{
			layout.init(
				{ 21, 1 },
				{ 2, 34 }
			);

			addAndMakeVisible(sortByName);
			addAndMakeVisible(sortByAuthor);

			makeTextButton(sortByName, "name");
			makeTextButton(sortByAuthor, "author");

			sortByName.onClick.push_back([&]()
				{
					sortByName.toggleState = sortByName.toggleState == 0 ? 1 : 0;

					auto sortFunc = [&ts = sortByName.toggleState](const SharedPatch& a, const SharedPatch& b)
					{
						if (ts == 1)
							return a->name.getText().compareNatural(b->name.getText()) > 0;
						else
							return a->name.getText().compareNatural(b->name.getText()) < 0;
					};

					patchList.sort(sortFunc);
				});

			sortByAuthor.onClick.push_back([&]()
				{
					sortByAuthor.toggleState = sortByAuthor.toggleState == 0 ? 1 : 0;

					auto sortFunc = [&ts = sortByAuthor.toggleState](const SharedPatch& a, const SharedPatch& b)
					{
						if (ts == 1)
							return a->author.getText().compareNatural(b->author.getText()) > 0;
						else
							return a->author.getText().compareNatural(b->author.getText()) < 0;
					};

					patchList.sort(sortFunc);
				});

			{
				auto& nLabel = sortByName.getLabel();
				auto& authLabel = sortByAuthor.getLabel();

				nLabel.textCID = ColourID::Hover;
				authLabel.textCID = nLabel.textCID;

				nLabel.font = getFontDosisMedium();
				authLabel.font = nLabel.font;

				nLabel.just = Just::centredLeft;
				authLabel.just = nLabel.just;

				nLabel.mode = Label::Mode::TextToLabelBounds;
				authLabel.mode = nLabel.mode;
			}

			addAndMakeVisible(patchList);
		}

		bool save(const String& _name, const String& _author)
		{
			return patchList.save(_name, _author);
		}

		void removeSelected()
		{
			patchList.removeSelected();
		}

		SharedPatch getSelectedPatch() const noexcept
		{
			return patchList.getSelectedPatch();
		}

		void show(const String& containedString)
		{
			patchList.show(containedString);
		}

		PatchList& getPatchList() noexcept { return patchList; }
	protected:
		PatchList patchList;
		Button sortByName, sortByAuthor;

		void resized() override
		{
			layout.resized();

			auto buttonArea = layout(0, 0, 1, 1);
			{
				auto x = buttonArea.getX();
				auto y = buttonArea.getY();
				auto w = buttonArea.getWidth() / 2.f;
				auto h = buttonArea.getHeight();

				sortByName.setBounds(BoundsF(x, y, w, h).toNearestInt());
				x += w;
				sortByAuthor.setBounds(BoundsF(x, y, w, h).toNearestInt());
			}

			layout.place(patchList, 0, 1, 2, 1, false);
		}

		void paint(Graphics&) override {}
	};

	struct TagsSelector :
		public CompScrollable
	{
		static constexpr float RelHeight = 10.f;

		struct Tag :
			public Button
		{
			Tag(Utils& u, const String& str = "") :
				Button(u, "Click on this tag to de/select it.")
			{
				makeTextButton(*this, str);
			}
		};

		TagsSelector(Utils& u, PatchList& patchList) :
			CompScrollable(u),
			tags()
		{
			layout.init(
				{ 21, 1 },
				{ 1 }
			);

			for (auto i = 0; i < patchList.numPatches(); ++i)
			{
				const auto patch = patchList[i];
				for(const auto& tag: patch->tags)
					addTag(tag);
			}
		}

		bool addTag(const String& id)
		{
			for (const auto& tag : tags)
				if (tag->getLabel().getText() == id)
					return false; // tag already exists

			tags.push_back(std::make_unique<Tag>(utils, id));
			addAndMakeVisible(*tags.back());
			resized();
			return true;
		}

		void paint(Graphics&) override {}

		std::vector<std::unique_ptr<Tag>> tags;

		void resized() override
		{
			layout.resized();

			layout.place(scrollBar, 1, 0, 1, 1, false);

			const auto bounds = layout(0, 0, 1, 1, false);

			const auto width = bounds.getWidth();
			const auto right = bounds.getRight();

			const auto thicc = utils.thicc;

			auto x = bounds.getX();
			auto y = bounds.getY() - yScrollOffset;
			const auto h = RelHeight * thicc;
			const auto w = h * 3.f;

			const auto numTags = static_cast<float>(tags.size());
			const auto numTagsPerRow = std::floor(width / w);
			actualHeight = numTags * h / numTagsPerRow;

			for (auto& tag : tags)
			{
				if (x + w > right)
				{
					x = 0.f;
					y += h;
				}

				tag->setBounds(BoundsF(x, y, w, h).toNearestInt());

				x += w;
			}
		}
	};

	struct PatchInspector :
		public Comp
	{
		struct Tag :
			public Button
		{
			Tag(Utils& u, const String& str) :
				Button(u, "Click on this tag to select it.")
			{
				makeToggleButton(*this, str);
			}
		};

		struct Tags :
			public CompScrollable
		{
			Tags(Utils& u) :
				CompScrollable(u),
				tags()
			{
				layout.init(
					{ 1 },
					{ 13, 1 }
				);
			}

			void addTag(const String& str)
			{
				tags.push_back(std::make_unique<Tag>(utils, str));
				addAndMakeVisible(*tags.back());
				resized();
			}

			std::vector<std::unique_ptr<Tag>> tags;

			void resized() override
			{
				layout.resized();

				layout.place(scrollBar, 0, 1, 1, 1, false);

				const auto tagsArea = layout(0, 0, 1, 1, false);

				// blabla layout tags
			}
		};

		PatchInspector(Utils& u, PatchList& _patchList) :
			Comp(u, "", CursorType::Default),
			patchList(_patchList),
			name(u, "Name: "),
			author(u, "Author: "),
			patch(nullptr),

			tags(u),
			addTag(u, "Click here to add a new tag!"),
			removeTag(u, "Click here to remove the selected tag!"),
			tagEditor(u, "Type a tag name!", "Enter tag..")
		{
			layout.init(
				{ 1, 13, 21, 2 },
				{ 1, 13, 13, 1 }
			);

			{
				addAndMakeVisible(name);
				addAndMakeVisible(author);

				name.mode = Label::Mode::TextToLabelBounds;
				name.just = Just::centredLeft;
				name.textCID = ColourID::Txt;
				name.font = getFontDosisBold();

				author.mode = name.mode;
				author.textCID = name.textCID;
				author.font = name.font;
				author.just = name.just;
			}

			{
				addAndMakeVisible(tags);
				addAndMakeVisible(addTag);
				addAndMakeVisible(removeTag);
				addChildComponent(tagEditor);

				makeTextButton(addTag, "+");
				makeTextButton(removeTag, "-");

				addTag.onClick.push_back([&]()
					{
						if (!tagEditor.isEnabled())
						{
							tagEditor.enable();
							resized();
						}
						else
						{
							tags.addTag(tagEditor.getText());
							tagEditor.disable();
							tagEditor.setVisible(false);
						}
					});
			}
			
		}

		void paint(Graphics& g) override
		{
			const auto thicc = utils.thicc;
			g.setColour(Colours::c(ColourID::Bg));
			g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(thicc), thicc);
		}

		void update()
		{
			auto sel = patchList.getSelectedPatch();
			if (patch == sel)
				return;

			patch = sel;
			if (patch != nullptr)
			{
				name.setText("Name: " + patch->name.getText());
				author.setText("Author: " + patch->author.getText());
			}
			
			repaintWithChildren(this);
		}

	protected:
		PatchList& patchList;
		Label name, author;
		SharedPatch patch;

		Tags tags;
		Button addTag, removeTag;
		TextEditor tagEditor;

		void resized() override
		{
			layout.resized();

			std::array<Label*, 2> labelPtr { &name, &author };

			for (auto l : labelPtr)
				l->mode = Label::Mode::TextToLabelBounds;

			layout.place(name, 1, 1, 2, 1, false);
			layout.place(author, 1, 2, 2, 1, false);

			{
				auto minHeight = labelPtr.front()->font.getHeight();
				for (auto i = 1; i < labelPtr.size(); ++i)
				{
					const auto& l = *labelPtr[i];
					const auto nHeight = l.font.getHeight();
					if (minHeight < nHeight)
						minHeight = nHeight;
				}

				for (auto l : labelPtr)
				{
					l->mode = Label::Mode::None;
					l->setMinFontHeight(minHeight);
				}
			}
			
			layout.place(tags, 2, 1, 1, 2, false);
			layout.place(addTag, 3, 1, 1, 1, true);
			layout.place(removeTag, 3, 2, 1, 1, true);
			layout.place(tagEditor, 2.2f, 1.2f, .6f, 1.6f, false);
		}
	};

	struct PatchBrowser :
		public CompScreenshotable,
		public Timer
	{
		PatchBrowser(Utils& u) :
			CompScreenshotable(u),
			Timer(),
			closeButton(u, "Click here to close the browser."),
			saveButton(u, "Click here to save this patch."),
			removeButton(u, "Click here to remove this patch."),
			searchBar(u, "Define a name or search for a patch.", "Init.."),
			patchList(u),
			tagsSelector(u, patchList.getPatchList()),
			inspector(u, patchList.getPatchList())
		{
			layout.init(
				{ 1, 2, 34, 2, 2, 1 },
				{ 1, 2, 8, 34, 5, 1 }
			);

			makeTextButton(closeButton, "X", false);
			closeButton.getLabel().mode = Label::Mode::TextToLabelBounds;
			closeButton.getLabel().textCID = ColourID::Abort;
			closeButton.onClick.push_back([&]()
			{
				setVisible(false);
			});

			makeTextButton(saveButton, "save", false);
			saveButton.getLabel().mode = Label::Mode::TextToLabelBounds;
			saveButton.onClick.push_back([&]()
			{
				if (searchBar.isNotEmpty())
				{
					if (patchList.save(searchBar.getText(), "user"))
					{
						searchBar.clear();
						repaintWithChildren(this);
					}
				}
			});

			searchBar.onReturn = saveButton.onClick.back();

			makeTextButton(removeButton, "rmv", false);
			removeButton.getLabel().textCID = ColourID::Abort;
			removeButton.onClick.push_back([&]()
			{
				patchList.removeSelected();
			});

			addAndMakeVisible(closeButton);
			addAndMakeVisible(saveButton);
			addAndMakeVisible(removeButton);
			addAndMakeVisible(searchBar);
			addAndMakeVisible(tagsSelector);
			addAndMakeVisible(patchList);
			addAndMakeVisible(inspector);

			onScreenshotFX.push_back([](Graphics& g, Image& img)
				{
					imgPP::blur(img, g, 7);

					auto bgCol = Colours::c(ColourID::Bg);
					
					if(bgCol.getPerceivedBrightness() < .5f)
						for (auto y = 0; y < img.getHeight(); ++y)
							for (auto x = 0; x < img.getWidth(); ++x)
								img.setPixelAt
								(x, y,
									img.getPixelAt(x, y)
									.withMultipliedSaturation(.4f)
									.withMultipliedBrightness(.4f)
								);
					else
						for (auto y = 0; y < img.getHeight(); ++y)
							for (auto x = 0; x < img.getWidth(); ++x)
								img.setPixelAt
								(x, y,
									img.getPixelAt(x, y)
									.withMultipliedSaturation(.4f)
									.withMultipliedBrightness(1.5f)
								);
				});
		}

		void setVisible(bool e) override
		{
			if (e)
			{
				notify(EvtType::BrowserOpened);
				takeScreenshot();
				Comp::setVisible(e);
				searchBar.enable();
				startTimerHz(12);
			}
			else
			{
				notify(EvtType::BrowserClosed);
				stopTimer();
				searchBar.disable();
				Comp::setVisible(e);
			}
		}

		void paint(Graphics& g) override
		{
			CompScreenshotable::paint(g);
			//g.fillAll(Colour(0xff000000));

			//g.setColour(Colour(0x44ffffff));
			//layout.paint(g);
		}

		void resized() override
		{
			CompScreenshotable::resized();

			layout.resized();

			layout.place(closeButton, 1, 1, 1, 1, true);
			layout.place(saveButton, 3, 1, 1, 1, true);
			layout.place(removeButton, 4, 1, 1, 1, true);

			layout.place(searchBar, 2, 1, 1, 1, false);
			layout.place(tagsSelector, 2, 2, 1, 1, false);
			layout.place(patchList, 1, 3, 4, 1, false);

			layout.place(inspector, 1, 4, 4, 1, false);
		}

		void timerCallback() override
		{
			patchList.show(searchBar.getText().toLowerCase());

			inspector.update();
		}

		String getSelectedPatchName() const
		{
			const auto patch = patchList.getSelectedPatch();
			if(patch != nullptr)
				return patch->name.getText();
			return "init";
		}

	protected:
		Button closeButton;
		Button saveButton, removeButton;
		TextEditor searchBar;
		PatchListSortable patchList;
		TagsSelector tagsSelector;
		PatchInspector inspector;
	};


	struct ButtonPatchBrowser :
		public Button
	{
		ButtonPatchBrowser(Utils& u, PatchBrowser& _browser) :
			Button(u, "Click here to open the patch browser."),
			browser(_browser)
		{
			makeTextButton(*this, browser.getSelectedPatchName(), false);
			onClick.push_back([&]()
			{
				const auto e = browser.isVisible();
				if(e)
					browser.setVisible(false);
				else
				{
					browser.setVisible(true);
				}
			});
		}

	protected:
		PatchBrowser& browser;
	};

}