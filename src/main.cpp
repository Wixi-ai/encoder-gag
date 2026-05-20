#include <so_5/all.hpp>
#include "agents/db_agent.hpp" // ← именно так

int main()
{
  so_5::launch([](so_5::environment_t &env)
               {
        auto db_mbox = env.introduce_coop([](so_5::coop_t& coop) {
            return coop.make_agent<db_agent_t>()->so_direct_mbox();
        });

        so_5::send<msg_hello>(db_mbox, "из main"); });
  return 0;
}
