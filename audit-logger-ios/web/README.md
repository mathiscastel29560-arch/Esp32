# Audit Logger - Progressive Web App (PWA)

Application web iOS pour tests de sécurité sans fil - 100% gratuit, sans App Store!

## 📱 Installation sur iPhone

### Option 1: Installation Directe (Recommandée)

#### Si vous hébergez l'app sur un serveur:

1. **Ouvrez Safari** sur votre iPhone
2. **Allez à l'adresse** de votre serveur (ex: `https://votre-domaine.com/web/`)
3. **Appuyez sur le bouton de partage** (carré avec flèche en haut à droite)
4. **Sélectionnez "Sur l'écran d'accueil"**
5. **Nommez l'app** (ex: "Audit Logger")
6. **Appuyez sur "Ajouter"**

→ L'app apparaîtra sur votre écran d'accueil comme une vraie app! 🎉

### Option 2: Hébergement Local

#### Sur Mac/Linux:

```bash
# Naviguez dans le dossier web
cd audit-logger-ios/web

# Démarrez un serveur simple
python3 -m http.server 8000

# ou avec Node.js:
npx http-server
```

Puis accédez à `http://localhost:8000` depuis le navigateur sur votre iPhone.

#### Sur Windows:

```bash
# Avec Python
python -m http.server 8000

# ou téléchargez un serveur simple (ex: HFS)
```

## 🌐 Hébergement Gratuit

### GitHub Pages (Recommandé)

1. **Créez un repo public** sur GitHub
2. **Placez le dossier `web/` à la racine**
3. **Allez à Settings → Pages**
4. **Sélectionnez "Deploy from branch: main"**
5. **L'app sera à**: `https://votre-username.github.io/repo-name/web/`

### Autres Options Gratuites:
- **Netlify** - Drag & drop gratuit
- **Vercel** - Déploiement auto depuis GitHub
- **Firebase Hosting** - Gratuit pour petits projets
- **Heroku** - Gratuit (limité)

## 🔗 Connexion ESP32

1. **Ouvrez l'app** depuis votre écran d'accueil
2. **Allez à ⚙️ Paramètres**
3. **Entrez l'adresse IP** de votre ESP32
4. **Entrez le port** (ex: 8080)
5. **Appuyez sur 🔌 Connecter**
6. **L'app affichera** 🟢 **Connecté**

## 📊 Fonctionnalités

### 5 Onglets Principaux:
- **📊 Tableau de Bord** - Vue d'ensemble et statistiques
- **📋 Audits** - Gestion des projets d'audit
- **⚡ Attaques** - Configuration et lancement des tests
- **📈 Résultats** - Analyse des résultats
- **⚙️ Paramètres** - Connexion ESP32 et configuration

### Avantages PWA:
✅ **Gratuit complètement**  
✅ **Installation directe** (pas d'App Store)  
✅ **Fonctionne hors-ligne** (stockage local)  
✅ **Mise à jour automatique**  
✅ **Plein écran** (sans barre Safari)  
✅ **Icône sur l'écran d'accueil**  

## 🖥️ Structure des Fichiers

```
web/
├── index.html        # Application web complète
├── manifest.json     # Configuration PWA
├── sw.js            # Service Worker (offline)
└── README.md        # Ce fichier
```

## 💾 Stockage de Données

- **Audits et résultats** sauvegardés localement
- **Données synchronisées** via WebSocket avec ESP32
- **Pas d'envoi vers le cloud** (sauf si configuré)

## 🔌 WebSocket ESP32

L'app peut se connecter en temps réel à votre ESP32:

```
WebSocket URL: ws://[IP]:[PORT]/ws
Protocole: JSON
```

**Format message reçu:**
```json
{
  "type": "attack_result",
  "attackType": "Déauthentification",
  "target": "AA:BB:CC:DD:EE:FF",
  "duration": 30,
  "success": true,
  "message": "Attaque réussie"
}
```

## 🛠️ Configuration Avancée

### Modifer le serveur WebSocket:
Éditez dans `index.html`:
```javascript
const wsUrl = `ws://${esp32IP}:${esp32Port}/ws`;
```

### Ajouter des nouvelles attaques:
Dans la fonction `renderAttacks()`, ajoutez:
```javascript
{ name: '🆕 Ma Nouvelle Attaque', type: 'Cible', icon: '🟢' }
```

## 📲 Conseils d'Utilisation

### Premier Démarrage:
1. Créez un audit de test
2. Lancez une attaque de test
3. Consultez les résultats dans l'onglet Résultats
4. Configurez la connexion ESP32 dans Paramètres

### Mode Offline:
- Les données sont sauvegardées localement
- Actif par défaut dans Paramètres
- L'app fonctionne sans connexion réseau

### Sauvegarder les Données:
Les données sont sauvegardées automatiquement. Pour exporter:
1. Ouvrez la console du navigateur (F12)
2. Exécutez: `console.log(localStorage.getItem('auditLoggerData'))`
3. Copiez les données JSON

## 🐛 Dépannage

### "L'app n'apparaît pas sur l'écran d'accueil"
- Assurez-vous d'utiliser **HTTPS** (pas HTTP)
- Vérifiez que le `manifest.json` est valide
- Rechargez la page et réessayez

### "Pas de connexion à l'ESP32"
- Vérifiez l'adresse IP et le port
- Assurez-vous que l'ESP32 est alimenté et connecté au réseau
- Vérifiez les logs WebSocket dans la console (F12)

### "Les données ne se sauvegardent pas"
- Vérifiez que le localStorage n'est pas désactivé
- Libérez de l'espace de stockage
- Essayez de réinitialiser l'app dans Paramètres

## 📝 Support

Pour des questions ou des bugs:
1. Consultez la console du navigateur (F12)
2. Vérifiez les logs WebSocket
3. Testez avec le mode Mock (Paramètres)

## 🔐 Sécurité

- ✅ Données stockées localement uniquement
- ✅ Connexion WebSocket chiffrée (si HTTPS + WSS)
- ✅ Aucun envoi vers des serveurs externes
- ✅ Parfait pour les tests de pénétration autorisés

## 📄 License

Propriétaire - Audit Logger

---

**Status**: ✅ Progressive Web App complète et prête à l'utilisation

**Dernière mise à jour**: 2026-09-21

**Compatible avec**: iPhone 6+ et plus récents (iOS 11+)
