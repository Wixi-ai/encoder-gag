#include <so_5/all.hpp>
#include "agents/db_agent.hpp"

int main()
{
  so_5::launch([](so_5::environment_t &env)
               {
        // Создаём DB агента и получаем его почтовый ящик
        auto db_mbox = env.introduce_coop([](so_5::coop_t& coop) {
            return coop.make_agent<db_agent_t>()->so_direct_mbox();
        });

        // Отправляем сообщения
        so_5::send<msg_hello>(db_mbox, msg_hello{"из main"});
        so_5::send<msg_bye>(db_mbox, msg_bye{"from main"}); });
  return 0;
}
