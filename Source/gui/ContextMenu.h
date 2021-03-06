#pragma once
#include "Button.h"
#include "GUIParams.h"
#include "TextEditor.h"

namespace gui
{
	class ContextMenu :
		public CompWidgetable
	{
		Notify makeNotify(ContextMenu&);

	public:
		ContextMenu(Utils&);

		void init();

		void place(const Comp*);

		void paint(Graphics&) override;

		void addButton(String&& /*name*/, String&& /*tooltip*/);
		
		void setButton(const Button::OnClick&, int/*idx*/);

		std::vector<std::unique_ptr<Button>> buttons;
		std::vector<Label*> labelPtr;
	protected:
		PointF origin;
		BoundsF bounds;

		void resized() override;
	};

	class ContextMenuKnobs :
		public ContextMenu
	{
		Notify makeNotify2(ContextMenuKnobs& popUp);
	
	public:
		ContextMenuKnobs(Utils&);
	};

	class ContextMenuButtons :
		public ContextMenu
	{
		Notify makeNotify2(ContextMenuButtons& popUp);

	public:
		ContextMenuButtons(Utils&);
	};
}