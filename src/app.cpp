// // Copyright (c) 2024 DavidDeadly
// #include "app.h"
//
// #include <fcntl.h>
// #include <functional>
// #include <iostream>
// #include <libinput.h>
// #include <linux/input.h>
//
// #include <thread>
//
// App::~App() {
//   libinput_unref(this->input);
//   libinput_device_unref(this->device);
//
//   if (this->worker && this->worker->joinable())
//     this->worker->join();
// }
//
// void App::start() {
//   // auto handler = initLibinput(window);
//   // if (handler)
//   //   this->worker = new std::thread(handler);
// }
//
// // STACIC METHODS
// void App::handle_tablet_event(App *app,
//                               struct libinput_event_tablet_tool
//                               *tablet_event) {
//   double dx = libinput_event_tablet_tool_get_dx(tablet_event);
//   double dy = libinput_event_tablet_tool_get_dy(tablet_event);
//   // double pressure = libinput_event_tablet_tool_get_pressure(tablet_event);
//   //
//   // std::cout << "Pen movement: X=" << dx << ", Y=" << dy
//   //           << ", Pressure=" << pressure << std::endl;
//
//   bool tip_state = libinput_event_tablet_tool_get_tip_state(tablet_event);
//   app->isNotDrawing = tip_state == LIBINPUT_TABLET_TOOL_TIP_UP;
//
//   // NOTE: Absolute drawing
//   // double xpos = libinput_event_tablet_tool_get_x(tablet_event);
//   // double ypos = libinput_event_tablet_tool_get_y(tablet_event);
//   // double appX = (xpos / 156) + app->prevX + dx * 1920;
//   // double appY = (ypos / 101.0) + app->prevY + dy * 1080;
//
//   // NOTE: relative drawing
//   // Calculate new position using deltas
//   double scaleFactor = 0.2; // Reduce speed by scaling the deltas
//   double appX = app->prevX + dx * scaleFactor * (1920 / 156);
//   double appY = app->prevY + dy * scaleFactor * (1080 / 101);
//
//   // Clamp new position to window boundaries
//   appX = std::max(0.0, std::min(appX, 1920.0));
//   appY = std::max(0.0, std::min(appY, 1080.0));
//
//   // bool isValidLine = app->prevX >= 0 && app->prevY >= 0;
//   bool isDrawing = !app->isNotDrawing;
//   if (isDrawing) {
//     app->lines.push_back({app->prevX, app->prevY, appX, appY});
//
//     std::cout << "Drawing with pen at: " << appX << ", " << appY <<
//     std::endl;
//   }
//
//   app->prevX = appX;
//   app->prevY = appY;
// }
//
// std::function<void()> App::handle_input_events(GLFWwindow *window) {
//   return [window]() {
//     auto threadId = std::this_thread::get_id();
//     std::cout << "Listening for pen input on thread..." << threadId
//               << std::endl;
//
//     App *app = static_cast<App *>(glfwGetWindowUserPointer(window));
//     std::cout << "app: " << &app << std::endl;
//
//     while (libinput_dispatch(app->input) == 0) {
//       struct libinput_event *event;
//
//       for (; (event = libinput_get_event(app->input)) != NULL;
//            libinput_event_destroy(event)) {
//         auto type = libinput_event_get_type(event);
//
//         if (type != LIBINPUT_EVENT_TABLET_TOOL_AXIS)
//           continue;
//
//         auto *tablet_event = libinput_event_get_tablet_tool_event(event);
//         app->handle_tablet_event(app, tablet_event);
//       }
//     }
//   };
// }
//
// std::function<void()> App::initLibinput(GLFWwindow *window) {
//   static struct libinput_interface interface = {
//       [](const char *path, int flags, void *user_data) {
//         return open(path, flags);
//       },
//       [](int fd, void *user_data) { close(fd); }};
//
//   struct libinput *input = libinput_path_create_context(&interface, NULL);
//
//   if (!input) {
//     std::cerr << "Failed to initialize libinput context" << std::endl;
//     return NULL;
//   }
//
//   // TODO: get pen path programmatically
//   const char *path = "/dev/input/event4";
//   struct libinput_device *device = libinput_path_add_device(input, path);
//   if (!device) {
//     std::cerr << "Failed to initialize input device" << std::endl;
//     libinput_unref(input);
//     return NULL;
//   }
//
//   App *app = static_cast<App *>(glfwGetWindowUserPointer(window));
//   app->input = input;
//   app->device = device;
//
//   auto handler = handle_input_events(window);
//
//   return handler;
// }
