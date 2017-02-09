void ble_server_app_main(void);
void blink_app_main();
void app_main()
{
  blink_app_main();
  ble_server_app_main();
}
