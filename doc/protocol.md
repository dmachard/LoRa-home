# Spécification du Protocole LoRa (Binaire)

Ce document décrit le format binaire de transmission et d'encapsulation des données échangées entre les **Nœuds Capteurs** (ESP32-C3) et la **Passerelle** (ESP32-C6).

Le protocole a été optimisé pour minimiser la taille des trames radio (bande passante LoRa réduite) tout en assurant l'authentification et le chiffrement des données.

---

## 1. Structure Globale de la Trame Radio

Chaque paquet envoyé sur les ondes LoRa possède la structure binaire suivante :

| Offset (octets) | Champ | Taille (octets) | Description | Visibilité |
|---|---|---|---|---|
| `0` | Header (Entête) | `9` | Identifiant, séquence, aléa. Utilisé comme données associées authentifiées (AAD). | En clair |
| `9` | Payload (Données) | `43` | Structure `SensorPayload` chiffrée. | Chiffré |
| `52` | Auth Tag (Signature) | `8` | Code d'authentification du message (MAC) généré par AES-GCM. | En clair |

**Taille totale d'une trame standard :** $9 + 43 + 8 = 60 \text{ octets}$.

---

## 2. Structure de l'Entête (Header)

L'entête de 9 octets sert à identifier l'émetteur, empêcher le rejeu et construire le vecteur d'initialisation (IV) AES-GCM :

```c
// Structure logique de l'entête
struct Header {
  uint8_t  node_id;        // ID unique du nœud (de 1 à 15)
  uint32_t seq;            // Numéro de séquence (incrémenté à chaque envoi)
  uint32_t random_id;      // ID aléatoire généré au démarrage par le TRNG ESP32
};
```

*   **`node_id` (1 octet)** : Permet à la passerelle d'identifier instantanément le nœud et de charger la clé AES correspondante.
*   **`seq` (4 octets - Big Endian)** : Compteur de paquets. Utilisé pour empêcher les attaques par rejeu.
*   **`random_id` (4 octets - Big Endian)** : Entropie matérielle issue de `esp_random()`. Assure la fraîcheur de l'IV (nonce) même si le nœud redémarre et réinitialise son compteur de séquence `seq` à zéro.

---

## 3. Structure des Données Chiffrées (`SensorPayload`)

La structure `SensorPayload` fait **43 octets** et utilise l'attribut `packed` pour interdire tout alignement d'octets superflu (padding) par le compilateur GCC :

```cpp
struct SensorPayload {
  uint8_t count;               // Nombre de mesures présentes (max 6)
  SensorReading readings[6];   // Tableau statique de mesures (6 * 5B = 30B)
  uint8_t reset_reason;        // Raison du dernier redémarrage de l'ESP32
  uint8_t error_code;          // Code d'erreur système (0 = OK)
  uint16_t tx_interval;        // Intervalle de transmission (en secondes)
  char name[8];                // Nom du nœud (8 caractères, terminateur inclus)
} __attribute__((packed));
```

### Détail de `SensorReading` (5 octets) :
Chaque mesure individuelle est codée sur 5 octets :
```cpp
struct SensorReading {
  uint8_t type;       // Type de capteur / mesure (1 octet)
  int32_t value;      // Valeur entière signée sur 32 bits (4 octets)
} __attribute__((packed));
```

---

## 4. Identifiants des Capteurs (`ReadingType`)

Les types de mesures sont standardisés via l'énumération suivante :

| Valeur | Nom Enum | Capteur associé | Grandeur Physique | Règle d'échelle (Gateway) |
|---|---|---|---|---|
| `1` | `TYPE_DHT22_TEMP` | DHT22 | Température | Valeur brute / 100.0f (°C) |
| `2` | `TYPE_DHT22_HUM` | DHT22 | Humidité | Valeur brute / 100.0f (%) |
| `3` | `TYPE_AHT20_TEMP` | AHT20 | Température | Valeur brute / 100.0f (°C) |
| `4` | `TYPE_AHT20_HUM` | AHT20 | Humidité | Valeur brute / 100.0f (%) |
| `5` | `TYPE_BMP280_TEMP` | BMP280 | Température | Valeur brute / 100.0f (°C) |
| `6` | `TYPE_BMP280_PRES` | BMP280 | Pression atmosphérique | Valeur brute / 10.0f (hPa) |
| `7` | `TYPE_BH1750_LUX` | BH1750 | Luminosité | Valeur brute (Lux) |
| `8` | `TYPE_BATTERY` | Interne | Tension de batterie | Valeur brute (mV) |

---

## 5. Règle de Conversion des Valeurs (Scaling)

Afin d'éviter l'envoi de nombres à virgule flottante (`float`), gourmands en bande passante et complexes à standardiser d'une architecture à l'autre, les valeurs physiques réelles sont multipliées par un facteur fixe avant l'envoi, puis re-divisées par la passerelle :

*   **Températures** : Envoyées sous forme de `float * 100` (ex: $23.45 \text{ °C} \rightarrow 2345$).
*   **Humidités** : Envoyées sous forme de `float * 100` (ex: $45.67 \text{ \%} \rightarrow 4567$).
*   **Pressions** : Envoyées sous forme de `float * 10` (ex: $1013.25 \text{ hPa} \rightarrow 10132$ ou en Pa divisé par 10).
*   **Lux & Tension Batterie** : Envoyés bruts sans virgule (ex: $4200 \text{ mV} \rightarrow 4200$).
