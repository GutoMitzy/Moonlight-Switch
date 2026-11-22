#ifdef __SWITCH__
#include "joycon_players_view.hpp"

#include <borealis/platforms/switch/switch_input.hpp>
#include <array>
#include "Settings.hpp"

using namespace brls;
using namespace brls::literals;

JoyConPlayersView::JoyConPlayersView() : Box(Axis::COLUMN)
{
    getAppletFrameItem()->title = "Joy-Con: atribuir jogadores";
    setGrow(1);
    setPaddingLeft(24);
    setPaddingRight(24);

    auto* scroll = new ScrollingFrame();
    scroll->setGrow(1);
    addView(scroll);
    auto* cells = new Box(Axis::COLUMN);
    scroll->setContentView(cells);

    auto* info = new Header();
    info->setTitle("Atribuir Joy-Cons por controle físico");
    info->setSubtitle("Pressione A no Joy-Con desejado na linha do jogador. "
                      "Com um controle não mapeado, segure seu botão e toque na linha. "
                      "Use Esquecer para liberar um assento antigo.");
    cells->addView(info);

    auto* input = static_cast<SwitchInputManager*>(
        Application::getPlatform()->getInputManager());

    for (int seat = 0; seat < 4; ++seat)
    {
        auto* assign = new DetailCell();
        assign->setText("Atribuir Jogador " + std::to_string(seat + 1));
        assign->setDetailText("Pressione com o Joy-Con");
        cells->addView(assign);
        assign->registerClickAction([input, assign, seat](View*) {
            const bool assigned = input->assignLastActiveJoyConToSeat(seat);
            assign->setDetailText(assigned ? "Atribuído" :
                "Não identificado — pressione apenas o Joy-Con desejado");
            return true;
        });

        auto* forget = new DetailCell();
        forget->setText("Esquecer Jogador " + std::to_string(seat + 1));
        forget->setDetailText("Liberar posição");
        cells->addView(forget);
        forget->registerClickAction([input, forget, seat](View*) {
            if (input->forgetJoyConSeat(seat))
                forget->setDetailText("Posição liberada");
            return true;
        });
    }

    auto* clear = new DetailCell();
    clear->setText("Esquecer todos os jogadores deste modo");
    clear->setDetailText("Apaga apenas as atribuições do modo selecionado");
    cells->addView(clear);
    clear->registerClickAction([input, clear](View*) {
        input->forgetAllJoyConSeats();
        clear->setDetailText("Atribuições apagadas");
        return true;
    });
}
#endif
