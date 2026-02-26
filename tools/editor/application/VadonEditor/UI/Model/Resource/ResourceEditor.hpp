#ifndef VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEEDITOR_HPP
#define VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEEDITOR_HPP
#include <VadonEditor/UI/Model/Resource/ui_ResourceEditor.h>
namespace VadonEditor::UI
{
	class ResourceEditor : public QWidget
	{
	public:
		ResourceEditor(QWidget* parent);
	private:
		Ui::ResourceEditor m_ui;
	};
}
#endif