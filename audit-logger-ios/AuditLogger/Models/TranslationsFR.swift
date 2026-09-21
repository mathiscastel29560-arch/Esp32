import Foundation

struct TranslationsFR {
    static let translations: [String: String] = [
        // Window titles
        "main_window_title": "Audit Logger - Outils de Sécurité Sans Fil",
        "about_title": "À propos de Audit Logger",

        // Tab titles
        "tab_dashboard": "Tableau de Bord",
        "tab_audits": "Audits",
        "tab_attacks": "Attaques",
        "tab_results": "Résultats",
        "tab_settings": "Paramètres",

        // Menu bar
        "menu_file": "Fichier",
        "menu_edit": "Édition",
        "menu_view": "Affichage",
        "menu_tools": "Outils",
        "menu_help": "Aide",

        // File menu
        "action_new_audit": "Nouveau Audit",
        "action_open_audit": "Ouvrir Audit",
        "action_save_audit": "Enregistrer l'Audit",
        "action_export": "Exporter...",
        "action_exit": "Quitter",

        // Dashboard
        "label_metrics": "Métriques",
        "label_networks": "Réseaux",
        "label_devices": "Appareils",
        "label_packets": "Paquets",
        "label_alerts": "Alertes",
        "label_tool_status": "État des Outils",
        "label_event_timeline": "Chronologie des Événements",
        "label_network_discovery": "Découverte de Réseau",

        // Status indicators
        "status_connected": "Connecté",
        "status_connecting": "Connexion...",
        "status_error": "Erreur",
        "status_disconnected": "Déconnecté",
        "status_ready": "Prêt",
        "status_processing": "Traitement",
        "status_success": "Succès",
        "status_warning": "Attention",

        // Audit Manager
        "label_audit_name": "Nom de l'Audit",
        "label_audit_type": "Type d'Audit",
        "label_target": "Cible",
        "label_location": "Localisation",
        "label_operator": "Opérateur",
        "label_date_start": "Date de Début",
        "label_date_end": "Date de Fin",
        "label_status": "État",

        "button_new_audit": "➕ Nouvel Audit",
        "button_open": "Ouvrir",
        "button_delete": "Supprimer",
        "button_export": "Exporter",

        // Audit types
        "audit_type_wifi": "WiFi",
        "audit_type_ble": "Bluetooth LE",
        "audit_type_subghz": "Sub-GHz",
        "audit_type_nfc": "NFC",
        "audit_type_combined": "Combiné",

        // Attack panel
        "label_attack_type": "Type d'Attaque",
        "label_target_device": "Appareil Cible",
        "label_duration": "Durée",
        "label_intensity": "Intensité",
        "label_parameters": "Paramètres",

        "button_launch_attack": "🚀 Lancer Attaque",
        "button_stop_attack": "⏹️ Arrêter",
        "button_pause": "⏸️ Pause",
        "button_resume": "▶️ Reprendre",

        "attack_deauth": "Déauthentification",
        "attack_beacon_flood": "Inondation de Balises",
        "attack_ble_pin": "Brute Force PIN BLE",
        "attack_ble_gatt": "Lecture/Écriture GATT",
        "attack_subghz_replay": "Rejeu de Signal Sub-GHz",
        "attack_nfc_clone": "Clonage de Tag NFC",

        "attack_started": "Attaque lancée",
        "attack_completed": "Attaque terminée",
        "attack_stopped": "Attaque arrêtée",
        "attack_unauthorized": "Attaque non autorisée",

        // Results
        "label_results": "Résultats",
        "label_attack_id": "ID Attaque",
        "label_success": "Succès",
        "label_duration_seconds": "Durée (s)",
        "label_packets_sent": "Paquets Envoyés",
        "label_devices_affected": "Appareils Affectés",

        // Export
        "dialog_export": "Exporter Audit",
        "export_format_json": "JSON (Données Complètes)",
        "export_format_csv": "CSV (Feuille de Calcul)",
        "export_format_pdf": "PDF (Rapport Professionnel)",
        "export_success": "Audit exporté avec succès",

        // Buttons
        "button_ok": "OK",
        "button_cancel": "Annuler",
        "button_save": "Enregistrer",
        "button_close": "Fermer",
        "button_apply": "Appliquer",
        "button_reset": "Réinitialiser",

        // Messages
        "msg_no_audit_selected": "Aucun audit sélectionné",
        "msg_no_target": "Aucune cible sélectionnée",
        "msg_invalid_parameters": "Paramètres invalides",
        "msg_operation_successful": "Opération réussie",
        "msg_operation_failed": "Opération échouée",
        "msg_confirm_delete": "Confirmer la suppression de cet audit?",
        "msg_confirm_export": "Exporter les données de cet audit?",

        // Settings
        "settings_title": "Paramètres",
        "settings_language": "Langue",
        "settings_theme": "Thème",
        "settings_mock_mode": "Mode de Test (Mock)",
        "settings_auto_refresh": "Actualisation Automatique",
        "settings_esp32_connection": "Connexion ESP32",
        "theme_dark": "Sombre (Dracula)",
        "theme_light": "Clair",

        // Statistics
        "label_statistics": "Statistiques",
        "label_total_attacks": "Attaques Totales",
        "label_successful": "Réussies",
        "label_failed": "Échouées",

        // Keyboard shortcuts
        "shortcut_new_audit": "Ctrl+N",
        "shortcut_open_audit": "Ctrl+O",
        "shortcut_save": "Ctrl+S",
        "shortcut_export": "Ctrl+E",
        "shortcut_quit": "Ctrl+Q",
        "shortcut_refresh": "F5",

        // Error messages
        "error_title": "Erreur",
        "error_import_failed": "Échec de l'importation",
        "error_export_failed": "Échec de l'exportation",
        "error_esp32_connection": "Impossible de se connecter à l'ESP32",
        "error_database": "Erreur de base de données",
        "error_invalid_input": "Entrée invalide",

        // Success messages
        "success_title": "Succès",
        "success_attack_launched": "Attaque lancée avec succès",
        "success_data_imported": "Données importées avec succès",
        "success_data_exported": "Données exportées avec succès",

        // Welcome
        "welcome_title": "Bienvenue dans Audit Logger",
        "welcome_subtitle": "Outils de Test de Sécurité Sans Fil",
        "button_start": "Commencer",
        "button_tutorials": "Tutoriels",
    ]

    static func translate(_ key: String) -> String {
        return translations[key] ?? key
    }
}
