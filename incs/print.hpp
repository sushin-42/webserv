#ifndef PRINT_HPP
#define PRINT_HPP
#include "HttpConfig.hpp"

// void printConfig(HttpConfig &httpConfig)
// {
//     cout << BLUE("http") << endl;
//     cout << "index" << endl;
//     for (size_t t = 0; t < httpConfig.index.size(); t++)
//     {
//         cout << httpConfig.index[t] << " ";
//     }
//     cout << endl;
//     cout << "root" << endl;
//     cout << httpConfig.root << endl;
//     cout << "auto_index" << endl;
//     cout << httpConfig.auto_index << endl;
//     cout << "keepalive_requests" << endl;
//     cout << httpConfig.keepalive_requests << endl;
//     cout << "default_type" << endl;
//     cout << httpConfig.default_type << endl;
//     cout << "client_max_body_size" << endl;
//     cout << httpConfig.client_max_body_size << endl;
//     cout << "reset_timedout_connection" << endl;
//     cout << httpConfig.reset_timedout_connection << endl;
//     cout << "lingering_timeout" << endl;
//     cout << httpConfig.lingering_timeout << endl;
//     cout << "lingering_time" << endl;
//     cout << httpConfig.lingering_time << endl;
//     cout << "keepalive_time" << endl;
//     cout << httpConfig.keepalive_time << endl;
//     cout << "keepalive_timeout" << endl;
//     cout << httpConfig.keepalive_timeout << endl;
//     cout << "send_timeout" << endl;
//     cout << httpConfig.send_timeout << endl;
//     cout << "client_body_timeout" << endl;
//     cout << httpConfig.client_body_timeout << endl;
//     cout << BLUE("http") << endl;
//     for (size_t i = 0; i < httpConfig.link.size(); i++)
//     {
//         cout << BLUE("server") << endl;
//         cout << "index" << endl;

//         for (size_t t = 0; t < httpConfig.link[i]->index.size(); t++)
//         {
//             cout << httpConfig.link[i]->index[t] << endl;
//         }
//         cout << endl;
//         cout << "httpConfig.link[i]->root" << endl;
//         cout << httpConfig.link[i]->root << endl;
//         cout << "httpConfig.link[i]->auto_index" << endl;
//         cout << httpConfig.link[i]->auto_index << endl;
//         cout << "httpConfig.link[i]->keepalive_requests" << endl;
//         cout << httpConfig.link[i]->keepalive_requests << endl;
//         cout << "httpConfig.link[i]->default_type" << endl;
//         cout << httpConfig.link[i]->default_type << endl;
//         cout << "httpConfig.link[i]->client_max_body_size" << endl;
//         cout << httpConfig.link[i]->client_max_body_size << endl;
//         cout << "httpConfig.link[i]->reset_timedout_connection" << endl;
//         cout << httpConfig.link[i]->reset_timedout_connection << endl;
//         cout << "httpConfig.link[i]->lingering_timeout" << endl;
//         cout << httpConfig.link[i]->lingering_timeout << endl;
//         cout << "httpConfig.link[i]->lingering_time" << endl;
//         cout << httpConfig.link[i]->lingering_time << endl;
//         cout << "httpConfig.link[i]->keepalive_time" << endl;
//         cout << httpConfig.link[i]->keepalive_time << endl;
//         cout << "httpConfig.link[i]->keepalive_timeout" << endl;
//         cout << httpConfig.link[i]->keepalive_timeout << endl;
//         cout << "httpConfig.link[i]->send_timeout" << endl;
//         cout << httpConfig.link[i]->send_timeout << endl;
//         cout << "httpConfig.link[i]->client_body_timeout" << endl;
//         cout << httpConfig.link[i]->client_body_timeout << endl;
//         cout << BLUE("server") << endl;
//         for (size_t k = 0; k < httpConfig.link[i]->link.size(); k++)
//         {
//             cout << BLUE("location") << endl;
//             cout << "index" << endl;

//             for (size_t t = 0; t < httpConfig.link[i]->link[k]->index.size(); t++)
//             {
//                 cout << httpConfig.link[i]->link[k]->index[t] << endl;
//             }
//             cout << endl;
//             cout << "httpConfig.link[i]->link[k]->root" << endl;
//             cout << httpConfig.link[i]->link[k]->root << endl;
//             cout << "httpConfig.link[i]->link[k]->auto_index" << endl;
//             cout << httpConfig.link[i]->link[k]->auto_index << endl;
//             cout << "httpConfig.link[i]->link[k]->keepalive_requests" << endl;
//             cout << httpConfig.link[i]->link[k]->keepalive_requests << endl;
//             cout << "httpConfig.link[i]->link[k]->default_type" << endl;
//             cout << httpConfig.link[i]->link[k]->default_type << endl;
//             cout << "httpConfig.link[i]->link[k]->client_max_body_size" << endl;
//             cout << httpConfig.link[i]->link[k]->client_max_body_size << endl;
//             cout << "httpConfig.link[i]->link[k]->reset_timedout_connection" << endl;
//             cout << httpConfig.link[i]->link[k]->reset_timedout_connection << endl;
//             cout << "httpConfig.link[i]->link[k]->lingering_timeout" << endl;
//             cout << httpConfig.link[i]->link[k]->lingering_timeout << endl;
//             cout << "httpConfig.link[i]->link[k]->lingering_time" << endl;
//             cout << httpConfig.link[i]->link[k]->lingering_time << endl;
//             cout << "httpConfig.link[i]->link[k]->keepalive_time" << endl;
//             cout << httpConfig.link[i]->link[k]->keepalive_time << endl;
//             cout << "httpConfig.link[i]->link[k]->keepalive_timeout" << endl;
//             cout << httpConfig.link[i]->link[k]->keepalive_timeout << endl;
//             cout << "httpConfig.link[i]->link[k]->send_timeout" << endl;
//             cout << httpConfig.link[i]->link[k]->send_timeout << endl;
//             cout << "httpConfig.client_->ody_tim->out" << endl;
//             cout << httpConfig.link[i]->link[k]->client_body_timeout << endl;
//             LocationConfig *asdf = dynamic_cast<LocationConfig *>(httpConfig.link[i]->link[k]);
//             for (size_t a = 0; a < asdf->limit_except_method.size(); a++)
//                 cout << asdf->limit_except_method[a] << endl;
//             cout << BLUE("location") << endl;
//         }
//     }
// }
#endif