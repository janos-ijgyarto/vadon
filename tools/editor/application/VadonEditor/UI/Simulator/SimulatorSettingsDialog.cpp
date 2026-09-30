#include <VadonEditor/UI/Simulator/SimulatorSettingsDialog.hpp>

#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Simulator/Simulator.hpp>

#include <VadonEditor/UI/Model/Property/Property.hpp>

namespace VadonEditor::UI
{
	SimulatorSettingsDialog::SimulatorSettingsDialog(Core::Application& application, QWidget* parent)
		: QDialog(parent)
		, m_application(application)
	{
		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);
		m_ui.setupUi(this);

		Simulator::Simulator& simulator = m_application.get_simulator();
		if (simulator.is_running() == false)
		{
			qCritical() << "Simulator is not running!";
			return;
		}

		PropertyWidgetInfo widget_info;
		widget_info.type_list;

		const Simulator::SimulatorSettingsData& settings_data = simulator.get_settings();
		for (const Simulator::SimulatorSetting& current_setting : settings_data.settings)
		{
			widget_info.property_id = current_setting.id;

			widget_info.type_list.clear();
			widget_info.type_list.push_back(current_setting.type);

			widget_info.init_value = current_setting.value;

			PropertyWidget* current_setting_widget = PropertyWidget::create_widget(m_application, widget_info, this, nullptr);

			connect(current_setting_widget, &PropertyWidget::value_changed, this, &SimulatorSettingsDialog::setting_value_changed);

			QHBoxLayout* current_setting_hbox = new QHBoxLayout();
			QLabel* current_setting_label = new QLabel(current_setting.label);

			current_setting_hbox->addWidget(current_setting_label, 0);
			current_setting_hbox->addWidget(current_setting_widget, 1);

			m_ui.settingsScrollVBox->addLayout(current_setting_hbox);
		}
	}

	void SimulatorSettingsDialog::setting_value_changed(const QUuid& setting_id)
	{
		for (int item_index = 0; item_index < m_ui.settingsScrollVBox->count(); ++item_index)
		{
			QLayout* current_row = m_ui.settingsScrollVBox->itemAt(item_index)->layout();
			PropertyWidget* current_property_widget = qobject_cast<PropertyWidget*>(current_row->itemAt(1)->widget());
			if (current_property_widget == nullptr)
			{
				continue;
			}

			if (current_property_widget->get_id() == setting_id)
			{
				Simulator::Simulator& simulator = m_application.get_simulator();
				simulator.set_setting_value(setting_id, current_property_widget->get_value());
				return;
			}
		}
	}
}