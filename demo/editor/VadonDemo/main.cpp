#include <VadonDemo/Model/Model.hpp>
#include <VadonEditor/Core/Editor.hpp>

namespace
{
    class Editor : public VadonEditor::Core::Editor 
    {
    protected:
        bool post_init() override
        {
            // FIXME: need to do this because app and engine only start to exist after initialization
            // Might want to be able to decouple this?
            m_model = std::make_unique<VadonDemo::Model::Model>(get_engine_core());
            if (m_model->initialize() == false)
            {
                return false;
            }

            return true;
        }
    private:
        std::unique_ptr<VadonDemo::Model::Model> m_model;
    };
}

int main(int argc, char* argv[])
{
    Editor editor;
    return editor.execute(argc, argv);
}