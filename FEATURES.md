# FEATURES.md — Suite d'audit BLE

> Spécification fonctionnelle des modules BLE de l'outil.
> Voir `HARDWARE.md` pour la carte et le brochage.

---

## Cadre d'usage (à lire avant d'implémenter)

Cet outil est un **instrument d'audit de sécurité**, destiné à des tests **ciblés et autorisés** :
- reconnaissance passive de l'environnement radio,
- audit d'appareils **que l'on possède ou que l'on est explicitement autorisé à tester**,
- détection défensive d'attaques,
- test de robustesse **de ses propres équipements**, en **environnement RF isolé**.

**Hors périmètre, non implémenté :** toute émission BLE **indiscriminée** diffusée à l'environnement (flood d'advertising type « BLE spam » exploitant Continuity/Fast Pair/Swift Pair). Ce type de fonction touche des appareils tiers non consentants (téléphones de passants, voisins…), ne teste rien, et n'a pas sa place ici. Le module 4 ci-dessous en est la **contrepartie légitime** : un fuzzing dirigé vers **un seul appareil possédé**, en isolation.

---

## Fondation : NimBLE-Arduino

Toute la suite repose sur **NimBLE-Arduino** (`h2zero/NimBLE-Arduino`), la pile BLE légère — bien plus économe en RAM/flash que le Bluedroid d'origine. Choix important : le S3 fait déjà tourner le TFT, les deux radios, le GPS et l'IR.

---

## Architecture partagée

Les 4 fonctions sont **4 entrées de menu**, mais ne reposent que sur **2 briques BLE de base** :

| Brique de base | Utilisée par |
|---|---|
| **Moteur de scan** (observer, passif + actif) | Module 1 (recensement) + Module 3 (détection spam) |
| **Client GATT** (connexion + énumération) | Module 2 (audit GATT) + Module 4 (fuzzing) |

→ On écrit deux composants, les quatre features se posent dessus.

---

## Module 1 — Scan / recensement BLE

**Le socle.** Scan actif + passif. Pour chaque appareil détecté, collecter :
- adresse **MAC** + type (**aléatoire** ou **publique**),
- **RSSI**,
- **nom** annoncé,
- **UUID de services** annoncés,
- **manufacturer data** : les 2 premiers octets identifient le fabricant
  - `0x004C` = Apple, `0x0075` = Samsung, `0x00E0` = Google, etc.

**Affichage TFT :** liste triée par RSSI + vue détail par appareil.

---

## Module 2 — Audit GATT de ses propres appareils

Connexion en **client GATT** à une cible **autorisée**, puis énumération complète :
`services → caractéristiques → descripteurs`.

Pour chaque caractéristique : ses **propriétés** (read / write / notify) et si l'accès exige **chiffrement/appairage**.

**Points de sécurité remontés :**
- caractéristiques **lisibles sans appairage**,
- **écritures possibles sans authentification**,
- appairage en **Just Works** (pas de protection MITM),
- **fuites d'infos** — le *Device Information Service* expose souvent numéro de série et version firmware.

C'est le cœur du test de sécurité : à n'utiliser **que** sur ses équipements ou ceux qu'on est autorisé à auditer.

---

## Module 3 — Détecteur de BLE spam (défensif)

L'angle défensif. **Scan passif continu**, repérage des **signatures** de spam :
- fort taux de **beacons d'appairage** émis par une **multitude de MAC aléatoires différentes** :
  - Apple **Continuity**,
  - Google **Fast Pair** (service data `0xFE2C`),
  - Microsoft **Swift Pair**.

Au-delà d'un **seuil de beacons/seconde venant de MAC distinctes** → **alerte** : type détecté + RSSI le plus fort (pour orienter vers la source).

Utilité : savoir si **quelqu'un émet du spam BLE autour de soi**.

---

## Module 4 — Test de résilience de ses propres appareils

Fuzzing **ciblé**, sur **un seul appareil que l'on possède**, en **environnement RF isolé** (sac de Faraday, ou puissance minimale à très courte distance).

Connecté à **la** cible, on malmène son serveur GATT :
- **écritures surdimensionnées**,
- écritures sur des caractéristiques **read-only**,
- **cycles connexion/déconnexion rapides**.

On observe si la pile de la cible **tient**, **fuit de la mémoire**, ou **se fige**.

**Par construction, connecté à un seul appareil → n'affecte personne d'autre.** C'est ce qui le distingue d'un flood d'advertising (hors périmètre). À exécuter uniquement en isolation, sur du matériel à soi.

---

## Autres modules de l'outil (hors BLE — spec séparée à venir)

Pour mémoire, l'appareil porte aussi (voir `HARDWARE.md`) :
- **RF sub-GHz 433 MHz** (CC1101) — capture/analyse de signaux sub-GHz,
- **RF 2,4 GHz** (NRF24L01+PA/LNA),
- **IR** émission/réception (capture & rejeu de télécommandes),
- **GPS** (NEO-6M) — horodatage/position des relevés.

Ces jeux de fonctions feront l'objet de leur propre spec, dans le même cadre d'usage (ciblé, autorisé).
