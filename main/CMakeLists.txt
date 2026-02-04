idf_component_register(
    SRCS 
        "main.c"
        "display.c"
        "touchscreen.c"
        "menu.c"
        "utils.c"
        "settings.c"
        "data_logger.c"
        "memory_manager.c"
        "error_handler.c"
        "bmp_display.c"
        
        # Core attack modules
        "wifi_functions.c"
        "wifi_attacks_enhanced.c"
        "wifi_connect.c"
        "evil_twin.c"
        "wps_attack.c"
        "handshake_capture.c"
        
        "bluetooth_functions.c"
        "ble_attacks_enhanced.c"
        "ble_hid_attack.c"
        "ble_packet_capture.c"
        
        "rf_functions.c"
        "rf_attacks_enhanced.c"
        "rf_protocols_wrapper.c"
        "subghz_protocols.c"
        
        "ir_functions.c"
        "ir_raw_capture.c"
        
        "nfc_functions.c"
        "mifare_attacks.c"
        
        "gps_functions.c"
        "badusb_functions.c"
        "ducky_script.c"
        
        # UI and utilities
        "ui_simple.c"
        "ui_effects.c"
        "ui_dialogs.c"
        "stats.c"

        "scrollable_list.c"
        "tactile_browser.c"
        
        # System modules
        "file_manager.c"
        "sd_card.c"
        "packet_logger.c"
        "packet_capture.c"
        "signal_visualizer.c"
        "target_manager.c"
        "settings_menu.c"
        "attack_timer.c"
        
        # Advanced features
        "automation.c"
        "attack_profiles.c"
        "attack_scheduler.c"
        "stealth_mode.c"
        "signal_analyzer.c"
        "signal_display.c"
        "web_interface.c"
        "time_sync.c"
        
        # New features
        "sd_browser.c"
        "battery_monitor.c"
        "usb_msc.c"
        "led_alerts.c"
        "antenna_indicator.c"
        "brightness_control.c"
        "cc1101_driver.c"
        
        # FIND3 Indoor Positioning
        "find3_scanner.c"
        "find3_ui.c"
        
        # OUI-Spy
        "oui_spy.c"
        
        # Additional modules
        "stats_tracker.c"
        "confirmation_dialog.c"
        "rssi_display.c"
        
    INCLUDE_DIRS "."
)