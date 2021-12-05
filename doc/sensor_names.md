# `applesmc` Sensor Names and what they mean

All the names of the sensors from the applesmc are listed in the
`sensor_names.json` file. This file is used by the `macfanpp` program
to give better representation to user compared to the `lm_sensors`
output. They can also be used for establishing sensor groups when
specifying fan controls in `user.json`.

The names are taken from the following sources in the appendix sections and then
`sensor_names.json` is made by doing `sort | uniq` and then editing them to clean
up some of the duplicate and inconsistencies. And based one of the sources
that contained some possible naming rules, a few of the sensors were
labeled with "guess" in the name.

## Research on the sources

Ones marked with "(Guess)" are guesses based on the names in 
https://www.mail-archive.com/mactel-linux-devel@lists.sourceforge.net/msg00526.html

First source: https://discussions.apple.com/thread/4838014

    TA0P:         +36.5°C  AMBIENT_TEMPERATURE
    TA0p:         +36.5°C 
    TA1P:         +37.5°C  AMBIENT1_TEMPERATURE
    TA1p:         +37.5°C 
    TC0C:         +42.0°C  CPU_DIE_CORE_TEMPERATURE Digital
    TC0D:         +43.5°C  CPU_DIODE_TEMPERATURE Analog
    TC0E:         +43.8°C 
    TC0F:         +44.8°C 
    TC0G:         +99.0°C 
    TC0J:          +1.0°C 
    TC0P:         +40.2°C  CPU_PROXIMITY_TEMPERATURE
    TC0c:         +42.0°C 
    TC0d:         +43.5°C 
    TC0p:         +40.2°C 
    TC1C:         +42.0°C 
    TC1c:         +42.0°C 
    TC2C:         +42.0°C 
    TC2c:         +42.0°C 
    TC3C:         +42.0°C 
    TC3c:         +42.0°C 
    TCGC:         +41.0°C  CPU_PECI_CORE_TEMPERATURE
    TCGc:         +41.0°C  PECI_GFX_TEMPERATURE
    TCPG:        +103.0°C 
    TCSC:         +42.0°C  PECI_SA_TEMPERATURE


Other couple sources: 
- https://web.archive.org/web/20151103144947/http://www.parhelia.ch/blog/statics/k3_keys.html
- https://web.archive.org/web/20160525201657/http://jbot-42.github.io:80/Articles/smc.html
- https://superuser.com/questions/553197/interpreting-sensor-names specifically:

    PECI CPU                   : TCXC
    PECI CPU                   : TCXc
    CPU 1 Proximity            : TC0P
    CPU 1 Heatsink             : TC0H
    CPU 1 Package              : TC0D
    CPU 1                      : TC0E
    CPU 1                      : TC0F
    CPU Core 1                 : TC1C
    CPU Core 2                 : TC2C
    CPU Core 3                 : TC3C
    CPU Core 4                 : TC4C
    CPU Core 5                 : TC5C
    CPU Core 6                 : TC6C
    CPU Core 7                 : TC7C
    CPU Core 8                 : TC8C
    CPU 1 Heatsink Alt.        : TCAH
    CPU 1 Package Alt.         : TCAD
    CPU 2 Proximity            : TC1P
    CPU 2 Heatsink             : TC1H
    CPU 2 Package              : TC1D
    CPU 2                      : TC1E
    CPU 2                      : TC1F
    CPU 2 Heatsink Alt.        : TCBH
    CPU 2 Package Alt.         : TCBD
    PECI SA                    : TCSC
    PECI SA                    : TCSc
    PECI SA                    : TCSA
    PECI GPU                   : TCGC
    PECI GPU                   : TCGc
    GPU Proximity              : TG0P
    GPU Die                    : TG0D
    GPU Die                    : TG1D
    GPU Heatsink               : TG0H
    GPU Heatsink               : TG1H
    Memory Proximity           : Ts0S
    Mem Bank A1                : TM0P
    Mem Bank A2                : TM1P
    Mem Bank B1                : TM8P
    Mem Bank B2                : TM9P
    Mem Module A1              : TM0S
    Mem Module A2              : TM1S
    Mem Module B1              : TM8S
    Mem Module B2              : TM9S
    Northbridge Die            : TN0D
    Northbridge Proximity 1    : TN0P
    Northbridge Proximity 2    : TN1P
    MCH Die                    : TN0C
    MCH Heatsink               : TN0H
    PCH Die                    : TP0D
    PCH Die                    : TPCD
    PCH Proximity              : TP0P
    Airflow 1                  : TA0P
    Airflow 2                  : TA1P
    Heatpipe 1                 : Th0H
    Heatpipe 2                 : Th1H
    Heatpipe 3                 : Th2H
    Mainboard Proximity        : Tm0P
    Powerboard Proximity       : Tp0P
    Palm Rest                  : Ts0P
    BLC Proximity              : Tb0P
    LCD Proximity              : TL0P
    Airport Proximity          : TW0P
    HDD Bay 1                  : TH0P
    HDD Bay 2                  : TH1P
    HDD Bay 3                  : TH2P
    HDD Bay 4                  : TH3P
    Optical Drive              : TO0P
    Battery TS_MAX             : TB0T
    Battery 1                  : TB1T
    Battery 2                  : TB2T
    Battery                    : TB3T
    Power Supply 1             : Tp0P
    Power Supply 1 Alt.        : Tp0C
    Power Supply 2             : Tp1P
    Power Supply 2 Alt.        : Tp1C
    Power Supply 3             : Tp2P
    Power Supply 4             : Tp3P
    Power Supply 5             : Tp4P
    Power Supply 6             : Tp5P
    Expansion Slots            : TS0C
    PCI Slot 1 Pos 1           : TA0S
    PCI Slot 1 Pos 2           : TA1S
    PCI Slot 2 Pos 1           : TA2S
    PCI Slot 2 Pos 2           : TA3S
    CPU Core 1                 : VC0C
    CPU Core 2                 : VC1C
    CPU Core 3                 : VC2C
    CPU Core 4                 : VC3C
    CPU Core 5                 : VC4C
    CPU Core 6                 : VC5C
    CPU Core 7                 : VC6C
    CPU Core 8                 : VC7C
    CPU VTT                    : VV1R
    GPU Core                   : VG0C
    Memory                     : VM0R
    PCH                        : VN1R
    MCH                        : VN0C
    Mainboard S0 Rail          : VD0R
    Mainboard S5 Rail          : VD5R
    12V Rail                   : VP0R
    12V Vcc                    : Vp0C
    Main 3V                    : VV2S
    Main 3.3V                  : VR3R
    Main 5V                    : VV1S
    Main 5V                    : VH05
    Main 12V                   : VV9S
    Main 12V                   : VD2R
    Auxiliary 3V               : VV7S
    Standby 3V                 : VV3S
    Standby 5V                 : VV8S
    PCIe 12V                   : VeES
    Battery                    : VBAT
    CMOS Battery               : Vb0R
    CPU Core                   : IC0C
    CPU VccIO                  : IC1C
    CPU VccSA                  : IC2C
    CPU Rail                   : IC0R
    CPU DRAM                   : IC5R
    CPU PLL                    : IC8R
    CPU GFX                    : IC0G
    CPU Memory                 : IC0M
    GPU Rail                   : IG0C
    Memory Controller          : IM0C
    Memory Rail                : IM0R
    MCH                        : IN0C
    Mainboard S0 Rail          : ID0R
    Mainboard S5 Rail          : ID5R
    Misc. Rail                 : IO0R
    Battery Rail               : IB0R
    Charger BMON               : IPBR
    CPU Core 1                 : PC0C
    CPU Core 2                 : PC1C
    CPU Core 3                 : PC2C
    CPU Core 4                 : PC3C
    CPU Core 5                 : PC4C
    CPU Core 6                 : PC5C
    CPU Core 7                 : PC6C
    CPU Core 8                 : PC7C
    CPU Cores                  : PCPC
    CPU GFX                    : PCPG
    CPU DRAM                   : PCPD
    CPU Total                  : PCTR
    CPU Total                  : PCPL
    CPU Rail                   : PC1R
    CPU S0 Rail                : PC5R
    GPU Total                  : PGTR
    GPU Rail                   : PG0R
    Memory Rail                : PM0R
    MCH                        : PN0C
    PCH Rail                   : PN1R
    Mainboard S0 Rail          : PC0R
    Mainboard S0 Rail          : PD0R
    Mainboard S5 Rail          : PD5R
    Main 3.3V Rail             : PH02
    Main 5V Rail               : PH05
    12V Rail                   : Pp0R
    Main 12V Rail              : PD2R
    Misc. Rail                 : PO0R
    Battery Rail               : PBLC
    Battery Rail               : PB0R
    DC In Total                : PDTR
    System Total               : PSTR

- https://www.mail-archive.com/mactel-linux-devel@lists.sourceforge.net/msg00526.html


## Macbook Pro 14,2 sensor output from `lm_sensors`:

    applesmc-isa-0300
    Adapter: ISA adapter
    Left side  :  1253 RPM  (min = 6864 RPM, max = 6336 RPM)
    Right side  : 1352 RPM  (min = 6864 RPM, max = 6864 RPM)
    TA0V:          +24.5°C
    TB0T:          +35.5°C
    TB1T:          +32.5°C
    TB2T:          +35.5°C
    TBXT:          +35.5°C
    TC0E:          +47.0°C
    TC0F:          +49.5°C
    TC0P:          +43.2°C
    TC1C:          +47.0°C
    TC2C:          +46.0°C
    TCGC:          +46.0°C
    TCMX:          +47.0°C
    TCMc:           +0.0°C
    TCSA:          +45.0°C
    TCTD:           +0.0°C
    TCXC:          +46.5°C
    TH0A:          +34.8°C
    TH0B:           -0.2°C
    TH0C:           -0.2°C
    TH0F:          -45.2°C
    TH0R:          -45.2°C
    TH0a:          +34.8°C
    TH0b:           -0.2°C
    TH0c:           -0.2°C
    TH0x:         -127.0°C
    TM0P:          +42.8°C
    TPCD:          +45.0°C
    TSDO:           +0.0°C
    TSDV:         -127.0°C
    TTLD:          +50.5°C
    TTRD:          +48.5°C
    TW0P:         -127.0°C
    TaLC:          +39.5°C
    TaRC:          +36.5°C
    Th1H:          +43.0°C
    Th2H:          +41.0°C
    Ts0P:          +31.0°C
    Ts0S:          +36.0°C
    Ts1P:          +29.0°C
    Ts1S:          +40.5°C

