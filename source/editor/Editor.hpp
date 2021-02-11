#ifndef PROTOTYPE_ACTION_RPG_EDITOR_HPP
#define PROTOTYPE_ACTION_RPG_EDITOR_HPP


#include "Application.hpp"


namespace editor {

    class Editor : public core::Application {
    public:
        Editor();

        ~Editor();

        void init() override;

        void update() override;

        void draw() override;

        void cleanup() override;

        void menuBar();

        void entitiesPanel();

    private:
        size_t entitySelected = -1;
    };

} // namespace editor


#endif //PROTOTYPE_ACTION_RPG_EDITOR_HPP
