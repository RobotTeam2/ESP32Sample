void blink_app_main();
void signal_generator_app_main();
void ble_server_app_main(void);
void app_main()
{
  blink_app_main();
  signal_generator_app_main();
  ble_server_app_main();
}
