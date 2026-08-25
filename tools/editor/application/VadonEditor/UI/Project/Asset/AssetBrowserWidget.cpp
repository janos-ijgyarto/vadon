#include <VadonEditor/UI/Project/Asset/AssetBrowserWidget.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>

#include <QFileInfo>
#include <QPushButton>

namespace VadonEditor::UI
{
	AssetBrowserWidget::AssetBrowserWidget(QWidget* parent)
		: QWidget(parent)
		, m_application(nullptr)
	{
		m_ui.setupUi(this);
	}

	void AssetBrowserWidget::initialize(Core::Application& application, const QModelIndex& root_asset)
	{
		m_application = &application;

		m_ui.assetList->setModel(&application.get_asset_manager().get_model());

		connect(m_ui.assetList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AssetBrowserWidget::selection_changed);
		
		if (root_asset.isValid() == true)
		{
			Core::AssetManager& asset_manager = m_application->get_asset_manager();
			const Core::AssetInfo asset_info = get_asset_info(root_asset);
			Q_ASSERT_X(asset_info.is_valid() == true, "VadonEditor::UI::AssetBrowserWidget::initialize", "Invalid root index!");

			if (asset_info.type == Core::AssetType::FOLDER)
			{
				set_root(root_asset);
			}
			else
			{
				const QString parent_path = asset_info.get_parent_path();
				const QModelIndex root_index = asset_manager.find_asset_index_by_path(parent_path);

				set_root(root_index);
			}
		}
		else
		{
			set_root(QModelIndex());
		}
	}

	QModelIndex AssetBrowserWidget::get_selected_asset() const
	{
		const QModelIndexList selected_indexes = m_ui.assetList->selectionModel()->selectedIndexes();
		if (selected_indexes.isEmpty() == false)
		{
			return selected_indexes.first();
		}

		return QModelIndex();
	}

	Core::AssetInfo AssetBrowserWidget::get_asset_info(const QModelIndex& index) const
	{
		const Core::AssetManager& asset_manager = m_application->get_asset_manager();
		return asset_manager.get_asset_info(index);
	}

	QString AssetBrowserWidget::get_current_root_path() const
	{
		const Core::AssetInfo asset_info = get_asset_info(m_ui.assetList->rootIndex());
		if (asset_info.is_valid() == true)
		{
			return asset_info.path;
		}

		return "";
	}

	void AssetBrowserWidget::prev_clicked()
	{
		// TODO: use history!
	}

	void AssetBrowserWidget::next_clicked()
	{
		// TODO: use history!
	}

	void AssetBrowserWidget::up_clicked()
	{
		const QModelIndex current_root = m_ui.assetList->rootIndex();
		set_root(current_root.parent());
	}

	void AssetBrowserWidget::selection_changed(const QItemSelection& selected, const QItemSelection& deselected)
	{
		Q_UNUSED(selected);
		Q_UNUSED(deselected);

		const QModelIndex selected_index = get_selected_asset();
		const Core::AssetInfo asset_info = get_asset_info(selected_index);
		if (asset_info.is_valid() == true)
		{
			QFileInfo file_info(asset_info.path);
			m_ui.fileNameLineEdit->setText(file_info.fileName());
		}
		else
		{
			m_ui.fileNameLineEdit->clear();
		}

		emit(asset_selected(selected_index));
	}

	void AssetBrowserWidget::asset_double_clicked(const QModelIndex& index)
	{
		const Core::AssetInfo asset_info = get_asset_info(index);
		if (asset_info.type == Core::AssetType::FOLDER)
		{
			// Move to the selected folder
			set_root(index);
		}
		else
		{
			// Send out signal
			emit(asset_opened(index));
		}
	}

	void AssetBrowserWidget::name_line_edit_changed(const QString& text)
	{
		Q_UNUSED(text);
		update_controls();

		emit(file_name_changed(text));
	}

	void AssetBrowserWidget::set_root(const QModelIndex& root_index)
	{
		// Clear selection
		m_ui.assetList->selectionModel()->reset();

		const Core::AssetInfo asset_info = get_asset_info(root_index);

		m_ui.pathLineEdit->setText(asset_info.path.isEmpty() ? "<root>" : asset_info.path);

		m_ui.upButton->setEnabled(root_index.isValid());

		m_ui.assetList->setRootIndex(root_index);

		update_controls();
	}

	void AssetBrowserWidget::update_controls()
	{
		// TODO: anything?
	}
}