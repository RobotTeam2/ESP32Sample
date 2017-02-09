void app_ble_server_main(void);
void blink_app_main();
void app_main()
{
  blink_app_main();
  app_ble_server_main();
}
