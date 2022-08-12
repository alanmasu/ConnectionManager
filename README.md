# ConnectionManager.h

Libreria che gestisce connessione e riconnessione automatica alla rete WiFI, la connessione WPS, il LED di connessione.

Utilizza un LED (quello onboard di default) ed un pulsante (BOOT Button di default) per interfacciarsi con l'utente.

Può gestire un WebServer molto semplice che da la possibilità di sapere la versione software che è installata a bordo dell'ESP32 e di resettare tramite rete la scheda eseguendo o meno una reboot callback precedentemente ; gestisce l'aggiornamento Over the Air (OTA) aprendo una porta di rete visibile nell'IDE di Arduino.

In caso di errori e tempi prolungati di disconnessione esegue un reboot, secondo delle definite regole di reboot.

Questa libreria utilizza principalmente un LED ed un solo pulsante per interfacciarsi con l'utente che deve collegare la scheda al proprio WiFi.

## Connessione WPS

La connessione WPS è un metodo di connessione ottimo per le tecnologie IoT, infatti non prevede la necessità di dover inserire password e/o codici, basta solamente impostare il proprio router in modalità accoppiamento e abilitare lo stesso sulla scheda.

**Per utilizzare questa funzione è necessario impostare nel software la configurazione del WPS tramite [setWPSConfig](#setWPSConfig)**

Per connettere tramite WPS la scheda è necessario:

1. Attendere che dopo il boot il LED di connessione non sia lampeggiante (se si utilizzano le funzioni di connessione);
2. Abilitare il WPS sul proprio router;
3. Tenere premuto il pulsante di connessione fin che il LED di connessione incomincia a lampeggiare con semiperiodo di `WPSBlinkInterval`  che è possibile modificare tramite [`setWPSBlinkInterval`](#setWPSBlinkInterval);
4. Attendere che il LED resti fisso.

A questo punto la rete è memorizzata e se si utilizza [`startConnection`](#startConnection) non è necessario ripetere la procedura

## LED di connessione

Il LED di connessione è un ottimo modo di avere informazioni sullo stato di connessione della scheda al WiFi.

Il led ha principalmente 4 possibili modalità:

* OFF: la scheda non è connessa a nessuna rete,
* ON: la scheda è connessa ad una rete,
* LAMPEGGIANTE: è in corso un tentativo di connessione tramite WPS,
* LAMPEGGIANTE in modo asimmetrico: è in corso la prima connessione,
* LAMPEGGIANTE "a gruppi di due lampeggi": sta a significare che l'accoppiamento WPS è fallito od è finito il periodo di 120 secondi per portare a temine la connessione.

## Funzioni principali

### `startConnection`

```c++
virtual void startConnection(bool withWPS, bool tryReconnection)
```

Tenta la connessione all'ultima rete alla quale la scheda è stata connessa, esegue 8 tentativi a distanza di `WiFiIntervalTraConnetion` millisecondi (modificabile tramite [setReconnectTimeout](#setReconnectTimeout)), e se alla fine del processo non risulta connessa, viene abilitato il WPS e successivamente il parametro `tryReconnection` ne identifica il comportamento.

**Se si intende utilizzare il WPS deve essere preceduta da [setWPSConfig](#setWPSConfig) **

> Parametri: 

- withWPS: inizializza il WPS finita la procedura di connessione automatica, default: `true`
- tryReconnection: abilita la riconnessione automatica (e quindi i conteggi di reboot options) se il tentativo iniziale fallisce, default: `true`

> Return: `void`

***

### `startWiFi`

```c++
void startWiFi(const char ssid[], const char pass[], byte retries, bool bloc)
```

Esegue la connessione ad una specifica rete che viene settata mediante i parametri. 

> Parametri:

- ssid: C string che contiene il nome della rete a cui connettersi, 
- pass: C string che contiene la password della rete,
- retries: numero di nuovi tentativi da effettuare, default: `0` (ovvero esegue un tentativo solo)
- bloc: flag che permette di far rimanere bloccata la funzione fino al primo successo di connessione [DANGER ZONE], default: `false`

> Return: `void`

***

### `setWPSConfig`

```c++
void setWPSConfig(esp_wps_config_t *spech, bool isNotDefault)
```

Serve per settare la struct delle configurazioni per il WPS, se non viene eseguita allora il WPS non potrà essere utilizzato

> Parametri:

* spech: puntatore alla struttura
* isNotDefault: serve per riempire la struct con i campi di default, default: `false` (inizializza con i parametri di default)

> Return: `void`

***

### `loop`

```c++
virtual void loop(bool withServer = true, bool withOTA = true)
```

Gestisce le funzioni di riconnessione, di connessione tramite WPS, il lampeggio del LED di connessione e del pulsante di controllo, esegue l'aggiornamento dei componenti ausiliari come il WebServer e l'OTA updater.

> Parametri:

- whitServer: flag per eseguire l'handle del WebServer associato, default: `true`
- withOTA: flag per eseguire l'handle dell'OTA Updater, default: `true`

> Return: `void`

***

### `getState`

```c++
uint8_t getState() const
```

Restituisce lo stato della connessione

> Parametri: `void`

> Return: `uint8_t`

* FIRST_CONNECTION: Se è in corso la prima connessione
* CONNECTED: Se la scheda è connessa ad una rete
* DISCONNECTED: Se è stato connesso ad una rete e si è disconnesso
* NOT_CONNECTED: Se la prima connessione non è andata a buon fine
* WPS_CONNECTION: Se è in corso un tentativo di connessione mediante WPS
* WPS_TIMEOUT: Se dopo 120 secondi dalla pressione del tasto non si è ancora connesso la procedura termina e viene restituito questo valore
* WPS_FAILED: Se qualcosa va storto durante la procedura di accoppiamento WPS

***

### `getStringState`

```c++
virtual String getStringState()
```

Restituisce i valori di cui sopra in formato `String`

> Parametri: `void`

> Return: `String`

***

## Funzioni secondarie

### Getter

#### `getOTAHostname`

```c++
String getOTAHostname()const
```

Restituisce il nome di hosting sulla rete .local

> Parametri: `void`

> Return: `String`

Restituisce una stringa vuota se l'assegnamento non è andato a buon fine

***

#### `toString`

```c++
String toString()const
```

Restituisce una stringa dell'oggetto serializzato

---

### Setter

#### `setAutoReconnect`

```c++
void setAutoReconnect(bool en)
```

Serve per abilitare la riconnessione automatica, se combinata con `setRebootOptions`  può portare a resettare la scheda

> Parametri:

* en: flag che abilita o meno l'auto-riconnessione

> Return: `void`

***
#### `setReconnectTimeout`

```c++
void setReconnectTimeout(uint16_t time)
```

Setta il tempo tra una connessione e l'altra, va a modificare la proprietà `WiFiIntervalTraConnetion`

> Parametri:

* time: tempo in ms tra una connessione e l'altra

> Return: `void`

***
#### `setRebootOptions`

```c++
void setRebootOptions(bool forTime , bool forRetries, uint16_t max_time, uint32_t max_retries)
```

Serve per settare le opzioni di rebooting in caso di disconnessione prolugata

> Parametri:

* forTime: flag che permette di abilitare il reboot in caso la scheda sia disconnessa da una rete da più di `maxDisconnectedTime`, default: `REBOOT_ON_NOT_RECONNECTION_FOR_TIME`
* forRetries: flag che permette di abilitare il reboot in caso la scheda tenti di riconnettersi da più di `maxReconnetcRetries`, default: `REBOOT_ON_NOT_RECONNECTION_FOR_RETRIES`
* max_time: parametro che imposta `maxDisconnectedTime`, default: `MAX_DISCONNECTED_TIME`
* max_retries: parametro che imposta `maxReconnetcRetries`, default: `MAX_RECONNECTION_TIMES`

> Return: `void`

***

#### `setServer`

```c++
void setServer(WebServer *s, bool withHomepage = false)
```

Serve per passare il riferimento al WebServer, imposta le risposte alle seguenti richieste:

* /infoAbout viene visualizzata la versione
* /reboot viene eseguito un reboot dopo aver eseguito una Callback
* /onlyReboot viene eseguito un reboot senza eseguire nessuna Callback

> Parametri:

* s: Puntatore al WebServer già allocato
* whitHomepage: è un flag che permette di impostare una home page base:
  * alla richiesta di root risponde con questa pagina base

> Return: `void`

***

#### `setHomepage`

```c++
virtual void setHomepage()
```

Imposta una home page di default raggiungibile tramite l'IP oppure l'hostname di rete: //IP/ oppure //hostname.local/

> Parametri: `void`

> Return: `void`

***

#### `setDefaultWPSConfig`

```c++
void setDefaultWPSConfig()
```

Imposta i parametri di default nella configurazione del WPS

> Parametri: `void`

> Return: `void`

***

#### `setStaticIPAddress`

```c++
void setStaticIPAddress(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress DNS, IPAddress DNS2)
```

Seve per impostare staticamente i parametri di rete

> Parametri:

* ip: IPAddress contenete l'IP statico da assegnare
* gateway: IPAddress contenete l'IP del gateway
* subnet: IP della subnet mask
* DNS: indirizzo DNS primario
* DNS2: indirizzo DNS secondario

> Return: `void`

***

#### `setOnRebootCallback`

```c++
void setOnRebootCallback(void (*callback)(void)) 
```

Setta un puntatore a funzione che se impostato viene eseguito quando viene eseguito un reboot per problemi di connessione, utile per salvare i dati.

> Parametri:

* callback: puntatore a funzione `void callback(void)` 

> Return: `void`

***

#### `setWPSBlinkInterval`

```c++
void setWPSBlinkInterval(uint16_t time)
```

Serve per impostare la velocità di blink del LED di connessione durante la connessione WPS

> Parametri:

* time: semi periodo del lampeggio in ms

> Return: `void`

***

#### `setMaxInitialTimeout`

```c++
void setMaxInitialTimeout(uint16_t time)
```

Setta la proprietà `WiFiMaxInitialTimeout`, è il timeout massimo che può trascorrere per la connessione WiFi solo se si utilizza `startWiFi(ssid, pass, false)`

> Parametri:

* time: massimo timeout

> Return: `void`

***

### Configurators

#### `configLedPin`

```c++
void configLedPin(byte pin);
```

Serve per impostare il pin del LED di connessione, setta già la modalità di quello specifico GPIO ad OUTPUT

> Parametri:

* pin: il numero del GPIO che si intende usare come segnalazione del led di connessione

> Return: `void`

***

#### `configButton`

```c++
void configButton(byte pin, byte connPinMode, byte mode)
```

Serve per configurare il pin per il pulsante di controllo delle connessioni

> Parametri: 

* pin: numero del GPIO alla quale è collegato il pulsante
* connPinMode: modalità di configurazione del pin: può essere `INPUT`, `INPUT_PULLUP` o `INPUT_PULLDOWN`
* mode: serve per impostare la modalità di collegamento effettivo del pulsante e può essere: `PULLUP` o `PULLDOWN`

> Return: `void`

***

### Starter

#### `startOTA`

```c++
void startOTA()
```

Inizializza l'updater Over The Air (OTA)

> Parametri: `void`

> Return: `void`

***

#### `startWebServer`

```c++
void startWebServer()
```

Inizializza il WebServer

> Parametri: `void`

> Return: `void`
