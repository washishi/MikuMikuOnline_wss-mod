//
// Option.hpp
//

#pragma once

#include <boost/property_tree/json_parser.hpp>
#include "Base.hpp"
#include "../ManagerHeader.hpp"
#include "../ManagerAccessor.hpp"
#include "../ResourceManager.hpp"
#include "../ui/UISuper.hpp"

using namespace boost::property_tree;

namespace scene {

class OptionTabBase;
typedef std::shared_ptr<OptionTabBase> OptionTabPtr;

class OptionItemBase;
typedef std::shared_ptr<OptionItemBase> OptionItemPtr;

class Option : public Base {
    public:
        Option(const ManagerAccessorPtr& manager_accessor,
			const BasePtr& background_scene);
        ~Option();
        void Begin();
        void Update();
		void ProcessInput(InputManager*);
        void Draw();
        void End();

    private:
        void AsyncInitialize();

    private:
        ManagerAccessorPtr manager_accessor_;
        ConfigManagerPtr config_manager_;
        CardManagerPtr card_manager_;
        AccountManagerPtr account_manager_;

		BasePtr background_scene_;

        int start_count_;
        int end_count_;

		int bg_alpha_;
		Rect base_rect_;
		std::array<ImageHandlePtr,4> base_image_handle_;
		std::array<ImageHandlePtr,3> tab_base_image_handle_;

		std::vector<OptionTabPtr> tabs_;
		static int selecting_tab_index;

	private:
		static const int TAB_BLOCK_HEIGHT;
		static const int TAB_BLOCK_WIDTH;
};

class OptionTabBase {
	public:
		OptionTabBase(const tstring name,
			const ManagerAccessorPtr& manager_accessor);

		virtual void Update();
		virtual void ProcessInput(InputManager*);
		virtual void Draw();

		tstring name() const;
		void set_base_rect(const Rect& rect);

	protected:
		ManagerAccessorPtr manager_accessor_;
		
		Rect base_rect_;
		tstring name_;

		std::vector<OptionItemPtr> items_;
};

// ステータスタブ
class StatusTab : public OptionTabBase {
	public:
		StatusTab(const ManagerAccessorPtr& manager_accessor);
};

// 表示設定タブ
class DisplayTab : public OptionTabBase {
	public:
		DisplayTab(const ManagerAccessorPtr& manager_accessor);
};

// 操作設定タブ
class InputTab : public OptionTabBase {
	public:
		InputTab(const ManagerAccessorPtr& manager_accessor);
};

// その他タブ
class OtherTab : public OptionTabBase {
	public:
		OtherTab(const ManagerAccessorPtr& manager_accessor);
};

class OptionItemBase {
	public:
		OptionItemBase(const ManagerAccessorPtr& manager_accessor);

		virtual void Update() = 0;
		virtual void ProcessInput(InputManager*) = 0;
		virtual void Draw() = 0;

		virtual int height() const = 0;
		void set_base_rect(const Rect& rect);

	protected:
		ManagerAccessorPtr manager_accessor_;
		Rect base_rect_;
	
};

typedef std::shared_ptr<std::function<tstring(void)>> TextItemCallbackPtr;
class TextItem : public OptionItemBase {
	public:
		TextItem(const tstring& name,
			const TextItemCallbackPtr& callback,
			const ManagerAccessorPtr& manager_accessor);

		void Update();
		void ProcessInput(InputManager*);
		void Draw();
		int height() const;

	private:
		tstring name_;
		TextItemCallbackPtr callback_;
};

class DescriptionItem : public OptionItemBase {
	public:
		DescriptionItem(const tstring& text,
			const ManagerAccessorPtr& manager_accessor);

		void Update();
		void ProcessInput(InputManager*);
		void Draw();
		int height() const;

	private:
		tstring text_;
		TextItemCallbackPtr callback_;
};

typedef std::function<int(void)> RadioButtonItemGetter;
typedef std::function<void(int)> RadioButtonItemSetter;
typedef std::shared_ptr<RadioButtonItemGetter> RadioButtonItemGetterPtr;
typedef std::shared_ptr<RadioButtonItemSetter> RadioButtonItemSetterPtr;
class RadioButtonItem : public OptionItemBase {
	public:
		RadioButtonItem(const tstring& name,
			const tstring& items,
			const RadioButtonItemGetterPtr& getter,
			const RadioButtonItemSetterPtr& setter,
			const ManagerAccessorPtr& manager_accessor);

		void Update();
		void ProcessInput(InputManager*);
		void Draw();
		int height() const;

	private:
		struct Item{
			Item(tstring _name, int _value, int _width) :
				name(_name), value(_value), width(_width) {}
			tstring name;
			int value, width;
			bool hover;
		};

	private:
		tstring name_;
		std::vector<Item> items_;
		int selecting_index_;

		RadioButtonItemGetterPtr getter_;
		RadioButtonItemSetterPtr setter_;

		std::array<ImageHandlePtr,3> selecting_bg_image_handle_;
};



}