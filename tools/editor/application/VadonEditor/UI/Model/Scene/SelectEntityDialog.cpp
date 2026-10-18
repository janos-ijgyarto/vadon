#include <VadonEditor/UI/Model/Scene/SelectEntityDialog.hpp>

#include <VadonEditor/Model/Scene/Entity.hpp>
#include <VadonEditor/Model/Scene/Scene.hpp>

#include <QPushButton>

namespace VadonEditor::UI
{
	SelectEntityDialog::SelectEntityDialog(Model::Scene* owner_scene, QWidget* parent)
		: QDialog(parent)
		, m_owner_scene(owner_scene)
	{
		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

		m_ui.setupUi(this);

		Model::EntityModel& entity_model = owner_scene->get_entity_model();
		m_ui.sceneTree->setModel(&entity_model.get_qt_model());

		connect(m_ui.sceneTree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SelectEntityDialog::selection_changed);

		update_controls();
	}

	void SelectEntityDialog::selection_changed(const QItemSelection& selected, const QItemSelection& deselected)
	{
		Q_UNUSED(selected);
		Q_UNUSED(deselected);
		update_controls();
	}

	void SelectEntityDialog::entity_double_clicked(const QModelIndex& index)
	{
		Model::Entity* entity = get_entity(index);
		finalize_entity_selection(entity);
	}

	void SelectEntityDialog::ok_clicked()
	{
		const Model::Entity* selected_entity = get_selected_entity();
		finalize_entity_selection(selected_entity);
	}

	void SelectEntityDialog::update_controls()
	{
		QPushButton* ok_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok);

		const Model::Entity* selected_entity = get_selected_entity();
		if (selected_entity != nullptr)
		{
			ok_button->setEnabled(true);
		}
		else
		{
			ok_button->setEnabled(false);
		}
	}

	Model::Entity* SelectEntityDialog::get_selected_entity() const
	{
		QItemSelectionModel* selection_model = m_ui.sceneTree->selectionModel();
		if (selection_model->hasSelection() == false)
		{
			return nullptr;
		}

		const QModelIndexList selected_index_list = selection_model->selectedIndexes();
		if (selected_index_list.isEmpty() == true)
		{
			return nullptr;
		}

		const QModelIndex selected_index = selected_index_list.front();
		return get_entity(selected_index);
	}

	Model::Entity* SelectEntityDialog::get_entity(const QModelIndex& index) const
	{
		return m_owner_scene->get_entity_model().get_entity_by_model_index(index);
	}

	void SelectEntityDialog::finalize_entity_selection(const Model::Entity* entity)
	{
		if (entity == nullptr)
		{
			// TODO: error!
			return;
		}

		emit(entity_selected(entity->get_id()));
		accept();
	}
}