# 🎯 Development Workflow - Branche Centralisée

## Structure Git Actuelle

```
main (STABLE - production ready)
  ↓
develop (PRIMARY - tous les développements)
  ├── Security Fixes (Claude Haiku Phase 1-3) ✅
  └── WPA2/EAPOL Capture (Claude - en consolidation)
```

## 📋 Branches Actuelles

| Branche | Status | Contenu | Action |
|---------|--------|---------|--------|
| `main` | 🟢 Stable | Code de production | Ne pas modifier directement |
| `develop` | 🟡 Actif | Tous les développements | **BRANCHE CENTRALE** |
| `claude/github-review-9rtnb3` | ✅ Fusionné | Fixes de sécurité Phase 1-3 | Archive/Supprime |
| `claude/handshake-wpa2-eapol-capture-r6yp2u` | 🔄 À rebaser | Capture WPA2/EAPOL | À rebaser sur develop |

## 🔄 Workflow pour les Développeurs

### Pour TOUT nouveau travail:

```bash
# 1. Partir de develop (pas de main!)
git checkout develop
git pull origin develop

# 2. Créer une branche de feature
git checkout -b claude/my-feature

# 3. Faire les changements
git add .
git commit -m "..."

# 4. Pusher vers remote
git push -u origin claude/my-feature

# 5. Fusionner DANS develop (via PR ou merge)
git checkout develop
git merge claude/my-feature
git push origin develop
```

## ✅ Consolidation Complète

### Phase 1: Security Fixes (✅ DONE)
- 10 commits consolidés dans `develop`
- Tous les fichiers sécurisés
- SECURITY_FIXES.md et CREDENTIALS_SECURITY.md créés

### Phase 2: WPA2/EAPOL Integration (🔄 IN PROGRESS)
- Branche source: `claude/handshake-wpa2-eapol-capture-r6yp2u`
- Status: Doit être rebasée sur `develop`
- Action: Attendre l'autre Claude Code pour rebaser, OU
  - Faire un PR pour merger intelligemment

## 📌 Règles Obligatoires

1. ✅ **Tout part de `develop`**, pas de `main`
2. ✅ **Pas de commits directs sur `main`** - utiliser des PRs
3. ✅ **Pas de commits directs sur `develop`** - utiliser des branches de feature
4. ✅ **Les branches temporaires sont supprimées** après fusion
5. ✅ **Garder `main` toujours stable** pour déploiement

## 🎬 Déploiement

```
develop → Test/Review
   ↓ (PR approuvé)
main → Production Ready ✅
```

## 📞 Contact

- **Session GitHub Audit**: Fixes de sécurité ✅
- **Session WPA2/EAPOL**: À coordonner pour rebase

---

**Last Updated**: 2026-09-22
**Maintainer**: Claude Code (Consolidated Development)
