# Sécurité de la Liaison LoRa et Provisioning

Ce document décrit le modèle de sécurité mis en œuvre pour protéger les données transmises par les nœuds capteurs et assurer l'intégrité de la passerelle.

Le système s'appuie sur le chiffrement authentifié **AES-GCM-128**, la protection contre le rejeu et un mécanisme de configuration sécurisé via Bluetooth Low Energy (BLE).

---

## 1. Chiffrement Authentifié : AES-128-GCM

Le protocole utilise le mode **AES-GCM (Galois/Counter Mode)** avec une clé symétrique de 128 bits. 

Contrairement aux modes de chiffrement classiques (comme le mode CBC) qui n'assurent que la confidentialité, le mode GCM apporte deux garanties cryptographiques majeures :
1.  **Confidentialité** : Les lectures des capteurs sont chiffrées. Un attaquant écoutant les fréquences radio ne peut pas connaître les valeurs remontées.
2.  **Intégrité / Authenticité** : Un code d'authentification de message (Tag MAC) de **8 octets** (64 bits) est calculé sur l'ensemble du message. La passerelle rejette immédiatement tout message altéré en cours de route.

---

## 2. Construction de l'IV (Vecteur d'Initialisation)

L'un des impératifs absolus du mode GCM est qu'**un même couple (Clé, IV) ne doit JAMAIS être réutilisé** sous peine de briser la sécurité de l'algorithme (fuite du flux de clé XOR).

Pour garantir cette unicité (Nonce) sur des microcontrôleurs sujets à des extinctions et des redémarrages fréquents, l'IV de 12 octets (96 bits) est assemblé dynamiquement ainsi :

```
+-----------+----------------------+--------------------------+---------------------+
| node_id   | sequence_number      | node_random_id           | Padding             |
| (1 octet) | (4 octets)           | (4 octets)               | (3 octets)          |
+-----------+----------------------+--------------------------+---------------------+
```

*   **Identifiant unique (`node_id`)** : Empêche deux nœuds distincts utilisant éventuellement la même clé d'émettre avec le même IV.
*   **Numéro de séquence (`seq`)** : Incrémenté de 1 à chaque paquet envoyé. Assure un IV unique pour chaque paquet au sein d'une même session d'exécution.
*   **Identifiant de session aléatoire (`node_random_id`)** : Généré à chaque démarrage à l'aide du **générateur de nombres aléatoires matériel (TRNG)** de l'ESP32 via `esp_random()`. Même si le nœud subit une coupure d'alimentation (ce qui réinitialise `seq` à `0`), le nouvel IV démarrera avec un `node_random_id` totalement différent, éliminant le risque de collision d'IV.

---

## 3. Données Associées Authentifiées (AAD)

L'entête de 9 octets (`node_id` + `seq` + `random_id`) doit être transmise en clair pour que la passerelle puisse identifier le nœud émetteur et décoder le paquet.

Pour éviter qu'un attaquant ne manipule ces octets en clair (par exemple en falsifiant l'identifiant du nœud ou le numéro de séquence), l'entête est passée comme **Authenticated Associated Data (AAD)** au bloc AES-GCM :

```cpp
gcm.addAuthData(frame, HDR_SIZE);
```

Si le moindre bit de l'entête est modifié pendant la transmission, la vérification du tag GCM échouera côté passerelle et le paquet sera immédiatement jeté.

---

## 4. Protection Contre le Rejeu

Un attaquant pourrait intercepter une trame valide (par exemple une trame indiquant "Température : 25°C") et la réémettre plusieurs fois pour fausser les graphiques ou saturer la passerelle.

Pour parer à cela :
1.  Le numéro de séquence `seq` est transmis dans chaque paquet.
2.  La passerelle conserve en mémoire le dernier numéro de séquence valide traité pour chaque nœud (`nodes[node_id].seq`).
3.  Tout paquet arrivant avec un numéro de séquence inférieur ou égal au dernier reçu est **immédiatement ignoré**, car suspecté d'être un rejeu.

---

## 5. Renforcement de la Clé AES (Hardening)

Pour éviter la présence d'une clé AES par défaut inscrite en dur dans le code source (vulnérabilité critique en cas de vol de code source ou de décompilation du firmware) :
*   **Initialisation vierge (Zero-Init)** : Au tout premier démarrage du nœud capteur, la clé AES en mémoire NVM (Preferences) est initialisée à zéro (`memset(0)`).
*   **Blocage opérationnel** : Le nœud refuse de transmettre des données significatives tant qu'une clé AES non nulle n'a pas été configurée.

---

## 6. Provisioning Sécurisé via Bluetooth (BLE)

Pour déployer et provisionner un nouveau nœud :
1.  L'installateur physique appuie sur le bouton **BOOT** du nœud pendant le démarrage pour forcer l'entrée en **Mode Configuration BLE**.
2.  Le nœud démarre son service BLE (sécurisé par le protocole `NimBLE-DataPipe`).
3.  L'installateur se connecte au nœud via une interface Web Bluetooth sécurisée.
4.  L'interface permet de définir de manière sécurisée en local :
    *   La clé AES unique du nœud.
    *   L'identifiant du nœud (`node_id`).
    *   Le nom du nœud (`node_name`).
    *   L'intervalle de transmission LoRa.
5.  Les paramètres sont stockés dans la mémoire non volatile (NVM) de l'ESP32. Au redémarrage, le nœud charge sa clé unique et commence ses transmissions sécurisées.
