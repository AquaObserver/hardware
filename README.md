# AquaObserver - Hardware
Izvorni kod za ESP32 povezan s HC-SR04 za mjerenje razine vode u spremniku i komunikaciju putem MQTT protokola.

## Setup
Potrebno je podesiti potrebne parametre kako bi se uspješno povezali s MQTT brokerom i pravilno očitavali razinu vode.

Parametri:
- [ ] `#define PORT 5000` Port na kojem je MQTT broker dostupan

- [ ] `IPAddress server(192, 168, 1, 104)` IP adresa MQTT brokera

- [ ] `// MQTT Client Config` je potrebno preurediti tako da se "aqua1" u svim konstantama zamjeni s jedinstvenim imenom

- [ ] `// Pins used` dio je potrebno prilagoditi stvarnoj povezanosti komponenti

- [ ] `int timeBetweenMeasurements` vrijeme između dva očitanja razine, u milisekundama.

- [ ] `float criticalValue` početna vrijednost kritične razine vode, ispod koje se uključuje LED[^1].

- [ ] `int totalContainerDepth` pretstavlja dubinu spremnika u kojem mjerimo razinu vode[^1].

[^1]: Vrijednost je moguće i naknadno podesiti slanjem odgovarajućih MQTT poruka