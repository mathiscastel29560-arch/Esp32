"""
French translations for Audit Logger

Traductions français pour Audit Logger
"""

TRANSLATIONS_FR = {
    # Window titles / Titres de fenêtres
    "main_window_title": "Audit Logger - Outils de Sécurité Sans Fil",
    "about_title": "À propos de Audit Logger",

    # Menu bar / Barre de menu
    "menu_file": "Fichier",
    "menu_edit": "Édition",
    "menu_view": "Affichage",
    "menu_tools": "Outils",
    "menu_help": "Aide",

    # File menu / Menu Fichier
    "action_new_audit": "Nouveau Audit",
    "action_open_audit": "Ouvrir Audit",
    "action_save_audit": "Enregistrer l'Audit",
    "action_export": "Exporter...",
    "action_exit": "Quitter",

    # Edit menu / Menu Édition
    "action_undo": "Annuler",
    "action_redo": "Refaire",
    "action_preferences": "Préférences",

    # View menu / Menu Affichage
    "action_dashboard": "Tableau de Bord",
    "action_audit_manager": "Gestionnaire d'Audits",
    "action_events": "Événements",
    "action_refresh": "Actualiser",

    # Tools menu / Menu Outils
    "action_inject_mock": "Injecter Données de Test",
    "action_attack_panel": "Panneau d'Attaque",
    "action_emergency_stop": "ARRÊT D'URGENCE",

    # Help menu / Menu Aide
    "action_help": "Aide",
    "action_about": "À propos",

    # Dashboard / Tableau de Bord
    "label_metrics": "Métriques",
    "label_networks": "Réseaux",
    "label_devices": "Appareils",
    "label_packets": "Paquets",
    "label_alerts": "Alertes",

    "label_tool_status": "État des Outils",
    "label_event_timeline": "Chronologie des Événements",
    "label_network_discovery": "Découverte de Réseau",

    # Status indicators / Indicateurs d'état
    "status_connected": "Connecté",
    "status_connecting": "Connexion...",
    "status_error": "Erreur",
    "status_disconnected": "Déconnecté",

    # Audit Manager / Gestionnaire d'Audits
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

    "dialog_new_audit": "Créer un Nouvel Audit",
    "dialog_audit_created": "Audit créé avec succès",
    "dialog_audit_deleted": "Audit supprimé",

    # Audit types / Types d'audit
    "audit_type_wifi": "WiFi",
    "audit_type_ble": "Bluetooth LE",
    "audit_type_subghz": "Sub-GHz",
    "audit_type_nfc": "NFC",
    "audit_type_combined": "Combiné",

    # Network table / Tableau de réseau
    "column_ssid": "SSID",
    "column_bssid": "BSSID",
    "column_channel": "Canal",
    "column_signal": "Signal",
    "column_encryption": "Chiffrement",
    "column_vendor": "Fabricant",
    "column_last_seen": "Vu",

    # Event timeline / Chronologie des événements
    "event_type_beacon": "Balise WiFi",
    "event_type_probe": "Sonde",
    "event_type_auth": "Authentification",
    "event_type_data": "Données",
    "event_type_deauth": "Déauthentification",

    "event_severity_info": "Info",
    "event_severity_warning": "Attention",
    "event_severity_critical": "Critique",

    # Attack panel / Panneau d'attaque
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

    # Results / Résultats
    "label_results": "Résultats",
    "label_attack_id": "ID Attaque",
    "label_target": "Cible",
    "label_success": "Succès",
    "label_duration_seconds": "Durée (s)",
    "label_packets_sent": "Paquets Envoyés",
    "label_devices_affected": "Appareils Affectés",

    # Export / Exportation
    "dialog_export": "Exporter Audit",
    "export_format_json": "JSON (Données Complètes)",
    "export_format_csv": "CSV (Feuille de Calcul)",
    "export_format_pdf": "PDF (Rapport Professionnel)",
    "export_success": "Audit exporté avec succès",

    # Buttons / Boutons
    "button_ok": "OK",
    "button_cancel": "Annuler",
    "button_save": "Enregistrer",
    "button_close": "Fermer",
    "button_apply": "Appliquer",
    "button_reset": "Réinitialiser",

    # Messages / Messages
    "msg_no_audit_selected": "Aucun audit sélectionné",
    "msg_no_target": "Aucune cible sélectionnée",
    "msg_invalid_parameters": "Paramètres invalides",
    "msg_operation_successful": "Opération réussie",
    "msg_operation_failed": "Opération échouée",
    "msg_confirm_delete": "Confirmer la suppression de cet audit?",
    "msg_confirm_export": "Exporter les données de cet audit?",

    # Settings / Paramètres
    "settings_title": "Paramètres",
    "settings_language": "Langue",
    "settings_theme": "Thème",
    "settings_mock_mode": "Mode de Test (Mock)",
    "settings_auto_refresh": "Actualisation Automatique",
    "settings_refresh_interval": "Intervalle d'Actualisation (sec)",
    "settings_port": "Port Serial",
    "settings_baudrate": "Vitesse en Baud",

    "theme_dark": "Sombre (Dracula)",
    "theme_light": "Clair",

    # Workflow / Flux de travail
    "label_workflow": "Flux de Travail",
    "label_workflow_name": "Nom du Flux",
    "label_steps": "Étapes",
    "label_dependencies": "Dépendances",
    "label_progress": "Progression",

    "button_add_step": "➕ Ajouter Étape",
    "button_start_workflow": "▶️ Démarrer Flux",
    "button_pause_workflow": "⏸️ Pause Flux",
    "button_cancel_workflow": "✕ Annuler Flux",

    "workflow_pending": "En Attente",
    "workflow_running": "En Cours d'Exécution",
    "workflow_completed": "Terminé",
    "workflow_failed": "Échoué",

    # Authorization / Autorisation
    "label_authorization": "Autorisation",
    "label_scope": "Périmètre",
    "label_duration_minutes": "Durée (minutes)",
    "label_targets": "Cibles Autorisées",
    "label_attack_types": "Types d'Attaque Autorisés",

    "msg_scope_expired": "Le périmètre d'autorisation a expiré",
    "msg_scope_created": "Nouveau perimètre créé avec succès",

    # Statistics / Statistiques
    "label_statistics": "Statistiques",
    "label_total_attacks": "Attaques Totales",
    "label_successful": "Réussies",
    "label_failed": "Échouées",
    "label_by_protocol": "Par Protocole",
    "label_by_type": "Par Type",
    "label_duration": "Durée Totale",

    # Help / Aide
    "help_title": "Aide - Audit Logger",
    "help_getting_started": "Commencer",
    "help_create_audit": "Créer un Audit",
    "help_launch_attack": "Lancer une Attaque",
    "help_export_results": "Exporter les Résultats",
    "help_keyboard_shortcuts": "Raccourcis Clavier",

    # Keyboard shortcuts / Raccourcis clavier
    "shortcut_new_audit": "Ctrl+N",
    "shortcut_open_audit": "Ctrl+O",
    "shortcut_save": "Ctrl+S",
    "shortcut_export": "Ctrl+E",
    "shortcut_quit": "Ctrl+Q",
    "shortcut_refresh": "F5",

    # Error messages / Messages d'erreur
    "error_title": "Erreur",
    "error_import_failed": "Échec de l'importation",
    "error_export_failed": "Échec de l'exportation",
    "error_esp32_connection": "Impossible de se connecter à l'ESP32",
    "error_database": "Erreur de base de données",
    "error_invalid_input": "Entrée invalide",

    # Success messages / Messages de succès
    "success_title": "Succès",
    "success_attack_launched": "Attaque lancée avec succès",
    "success_data_imported": "Données importées avec succès",
    "success_data_exported": "Données exportées avec succès",

    # Time / Temps
    "time_seconds": "secondes",
    "time_minutes": "minutes",
    "time_hours": "heures",
    "time_just_now": "À l'instant",
    "time_ago": "il y a",

    # Database / Base de données
    "label_database": "Base de Données",
    "label_backup": "Sauvegarde",
    "button_backup_now": "💾 Sauvegarder Maintenant",
    "button_restore": "📂 Restaurer",

    # System / Système
    "label_system": "Système",
    "label_version": "Version",
    "label_author": "Auteur",
    "label_license": "Licence",

    # Welcome / Bienvenue
    "welcome_title": "Bienvenue dans Audit Logger",
    "welcome_subtitle": "Outils de Test de Sécurité Sans Fil",
    "welcome_intro": "Outils professionnels pour les audits de sécurité en pénétration testing",
    "button_start": "Commencer",
    "button_tutorials": "Tutoriels",
}

def translate(key: str, default: str = None) -> str:
    """Get French translation for a key"""
    result = TRANSLATIONS_FR.get(key, default or key)
    return result
