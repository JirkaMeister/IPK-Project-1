# IPK - Projekt 1

## Teorie
Cílem prvního projektu byla tvorba klienta komunikujícího se serverem. Komunikace na straně klienta je rozdělena do 4 částí. V první části je třeba zajistit tvorbu soketu pro komunikaci. Podle typu komunikace (UDP/TCP) volíme příslušný typ soketu. V další části je potřeba správně nastavit adresu serveru. Doménovou adresu je třeba převést na IPv4 adresu. Při použítí TCP je nyní nutné navázat spojení se serverem. Ve třetí části se na danou adresu a port posílá zadaná zpráva. V poslední části se přijímá a zpracovává odpověd serveru.

## Implementační detaily
* Celý projekt jsem psal v jazyce C a překládal pomocí přiloženého Makefilu.
* Pro UDP odesílanou zprávu jsem využil strukturu:
  
        typedef struct{
            uint8_t opcode;
            uint8_t payloadLength;
            char payload[MAXLINE];
        } message_t;
* Pro přijímanou UDP zprávu jsem využil obdobnou strukturu s přidaným atributem `u_int8_t status`.
* TCP zprávy (přijímané i odesílané) jsem zpracovával jako klasické řetězce s upravenou délkou.
* Při tvorbě UDP i TCP zprávy jsem bral v potaz maximální délku řádku 255 znaků.
* Při nečekaném ukončení (Ctrl + C, EOF) při TCP komunikaci dojde vždy k odeslání příkazu `BYE`, aby došlo ke korektnímu uzavření komunikace mezi klientem a serverem.

## Testování
Projekt jsem testoval jednoduchým způsobem - vytvořil jsem si vstupní soubor s mnoha různými testovacími hodnotami. Ty jsem následně nechal program zpracovat a pomocí sktriptu jsem porovnával výstup s mým referečním výstupem pro dané hodnoty. Testoval jsem také ručně, abych zajistil správné chování programu při neočekávaném ukončení.

## Vývoj programu
Program jsem kvůli technickým problému nevyvíjel na gitea, ale na githubu. Repozitář jsem po dokončení projektu migroval na gitea.