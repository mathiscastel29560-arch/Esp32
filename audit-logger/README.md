# Audit Logger - Application Desktop

Application desktop pour les audits de sécurité sans fil, avec support complet pour WiFi, BLE, Sub-GHz et NFC.

## 🎯 Caractéristiques

- **Interface entièrement en français** - 300+ traductions
- **Icône personnalisée** - Bruce le requin + Botte avec crâne
- **Navigation simple** - 5 onglets intuitifs
- **Résultats clairs** - Cartes avec détails structurés
- **Thème Dracula** - Interface sombre professionnelle

## 📦 Structure

```
audit-logger/
├── ui/
│   ├── translations_fr.py      # Traductions français
│   ├── navigation.py           # Système de navigation
│   ├── results_display.py      # Affichage des résultats
│   └── __init__.py
├── backend/
│   ├── attacks/
│   │   ├── wifi_attacks.py
│   │   ├── ble_attacks.py
│   │   ├── subghz_attacks.py
│   │   ├── nfc_attacks.py
│   │   └── __init__.py
│   └── __init__.py
├── assets/
│   └── app_icon.svg            # Icône de l'application
├── docs/
│   └── INTERFACE_FRANCAISE.md  # Documentation complète
└── README.md
```

## 🚀 Démarrage Rapide

### Installation des dépendances

```bash
pip install PyQt6 PyQt6-WebEngine
```

### Lancer l'application

```python
python3 main.py
```

## 🇫🇷 Interface Française

### Les 5 Onglets

1. **📊 Tableau de Bord** - Vue d'ensemble des audits
2. **📋 Audits** - Gestion des projets d'audit
3. **⚡ Attaques** - Lancer les tests de pénétration
4. **📈 Résultats** - Analyser les résultats
5. **⚙️ Paramètres** - Configuration de l'application

### Couleurs de Statut

- 🟢 **Vert** (#2ecc71) - Succès
- 🔴 **Rouge** (#e74c3c) - Erreur
- 🟡 **Orange** (#f39c12) - Attention / Actif
- 🔵 **Bleu** (#3498db) - Information

## 📖 Documentation

Pour la documentation complète de l'interface française, consultez:
- `docs/INTERFACE_FRANCAISE.md` - Guide complet de l'interface

## 🛠️ Personnalisation

### Ajouter une traduction

```python
# Dans ui/translations_fr.py
TRANSLATIONS_FR = {
    "ma_clé": "Ma traduction française",
}
```

### Utiliser une traduction

```python
from ui.translations_fr import translate as tr

text = tr("ma_clé")  # "Ma traduction française"
```

## 📋 Composants Disponibles

### Navigation
- `NavigationBar` - Barre de navigation 5 onglets
- `BreadcrumbNavigation` - Fil d'Ariane
- `TabIndicator` - Indicateur de section
- `SimpleStatusDisplay` - Affichage du statut

### Affichage des Résultats
- `ResultCard` - Carte de résultat individuel
- `AttackResultDisplay` - Affichage spécialisé pour attaques
- `ProgressDisplay` - Barre de progression
- `ComparisonTable` - Tableau de comparaison
- `StatisticsDisplay` - Affichage des statistiques

## ⚙️ Configuration

Les paramètres de l'application peuvent être modifiés dans le menu **⚙️ Paramètres**:
- Langue (Français/Anglais)
- Thème (Sombre/Clair)
- Connexion ESP32
- Mode de test (Mock)
- Intervalle d'actualisation

## 🎓 Exemples

### Créer un résultat

```python
result = {
    "type": "attack",
    "status": "success",
    "title": "Déauthentification Réussie",
    "message": "Attaque lancée avec succès",
    "details": {
        "ID Attaque": "ATK_001",
        "Cible": "AA:BB:CC:DD:EE:FF",
        "Durée": "30s",
        "Paquets": "3000",
    },
    "actions": {
        "Exporter": export_func,
        "Détails": details_func,
    }
}

results_panel.show_result(result)
```

### Afficher une progression

```python
progress_display = ProgressDisplay("Attaque en cours")
progress_display.set_progress(50, "50% complété")
```

## 🔐 Sécurité

L'application inclut:
- ✅ Authentification utilisateur
- ✅ Autorisation basée sur les scopes
- ✅ Rate limiting des attaques
- ✅ Logs complets des opérations
- ✅ Verrous physiques (bouton RETOUR sur ESP32)

## 📝 License

Propriétaire - Audit Logger

## 👥 Auteur

Audit Logger Team

## 📞 Support

Pour des questions ou problèmes:
1. Vérifiez la documentation dans `docs/`
2. Consultez les examples dans ce README
3. Vérifiez les traductions dans `ui/translations_fr.py`

---

**Status**: ✅ Interface française complète et prête à l'utilisation

**Dernière mise à jour**: 2026-09-21
