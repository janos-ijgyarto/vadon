#include <VadonEditor/UI/Model/Object/ObjectDialog.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <QPushButton>

namespace VadonEditor::UI
{
	NewObjectDialog::NewObjectDialog(Core::Application& application, const QUuid& base_type, QWidget* parent)
		: QDialog(parent)
		, m_filter_model(nullptr)
	{
		Q_ASSERT_X(Utilities::is_uuid_valid(base_type), "VadonEditor::UI::NewObjectDialog::NewObjectDialog", "Invalid type UUID");
		setAttribute(Qt::WA_DeleteOnClose, true);

		m_ui.setupUi(this);

		const Core::DataSchema& data_schema = application.get_project_manager().get_project_data_schema();
		const_cast<QStandardItemModel*>(&data_schema.get_qt_model());

		m_filter_model = new Core::TypeFilterModel(data_schema, this);
		m_filter_model->setSourceModel(const_cast<QStandardItemModel*>(&data_schema.get_qt_model()));
		m_filter_model->set_root_type(base_type);
		m_filter_model->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

		m_ui.typeTreeView->setModel(m_filter_model);

		const QModelIndex root_type_index = data_schema.find_type_index(base_type);
		if (root_type_index.isValid() == true)
		{
			const QModelIndex parent_index = root_type_index.parent();
			const QModelIndex filtered_parent_index = m_filter_model->mapFromSource(parent_index);
			m_ui.typeTreeView->setRootIndex(filtered_parent_index);
		}

		m_ui.typeTreeView->expandAll();

		connect(m_ui.typeTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &NewObjectDialog::selection_changed);

		update_controls();
	}

	void NewObjectDialog::type_double_clicked(const QModelIndex& index)
	{
		const QUuid selected_type = get_selected_type(index);
		finalize_selection(selected_type);
	}

	void NewObjectDialog::filter_text_changed(const QString& text)
	{
		m_filter_model->setFilterFixedString(text);
		m_ui.typeTreeView->expandAll();
	}

	void NewObjectDialog::selection_changed(const QItemSelection& selected, const QItemSelection& deselected)
	{
		Q_UNUSED(selected);
		Q_UNUSED(deselected);
		update_controls();
	}

	void NewObjectDialog::selection_accepted()
	{
		const QUuid selected_type = get_selected_type(get_current_selection());
		finalize_selection(selected_type);
	}

	void NewObjectDialog::update_controls()
	{
		QPushButton* ok_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok);

		const QUuid selected_type = get_selected_type(get_current_selection());
		if (Utilities::is_uuid_valid(selected_type) == true)
		{
			ok_button->setEnabled(true);
		}
		else
		{
			ok_button->setEnabled(false);
		}
	}

	QModelIndex NewObjectDialog::get_current_selection() const
	{
		const QModelIndexList selected_indexes = m_ui.typeTreeView->selectionModel()->selectedIndexes();
		if (selected_indexes.isEmpty() == false)
		{
			return selected_indexes.first();
		}

		return QModelIndex();
	}

	QUuid NewObjectDialog::get_selected_type(const QModelIndex& index) const
	{
		if (index.isValid() == true)
		{
			return m_ui.typeTreeView->model()->data(index, static_cast<Qt::ItemDataRole>(Core::TypeTreeDataRole::TYPE_UUID)).toUuid();
		}
		else
		{
			return QUuid();
		}
	}

	void NewObjectDialog::finalize_selection(const QUuid& type_uuid)
	{
		Q_ASSERT_X(Utilities::is_uuid_valid(type_uuid) == true, "VadonEditor::UI::NewObjectDialog::finalize_selection", "Must select a valid object type");

		emit(object_type_selected(type_uuid));
		accept();
	}
}