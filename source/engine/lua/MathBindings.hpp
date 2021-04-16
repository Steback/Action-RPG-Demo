#ifndef PROTOTYPE_ACTION_RPG_MATHBINDINGS_HPP
#define PROTOTYPE_ACTION_RPG_MATHBINDINGS_HPP


namespace sol {
    class state;
}

namespace engine::lua {

    void setMathBindings(sol::state& state);

}


#endif //PROTOTYPE_ACTION_RPG_MATHBINDINGS_HPP
