#ifndef __NUBIA_HW_VERSION_H__
#define __NUBIA_HW_VERSION_H__

#define PCB_VER_LENGTH 12
#define RF_BAND_LENGTH 12
#define WIFI_STRING_LENGTH 15
#define CONFIG_STRING_LENGTH 8


struct nubia_gpio_pinctrl_info {
        struct pinctrl *pinctrl;
        struct pinctrl_state *pull_up_default;
        struct pinctrl_state *pull_down_default;
};


typedef enum
{
	HW_A,
	HW_B,
    HW_B1,
	HW_B2,
	HW_B3,
	HW_C,
	HW_C1,
	HW_D,
	HW_E,
	HW_F,
	HW_G,
	HW_H,
	HW_I,
	HW_UNKNOW// unknow, fail read
}hw_pcb_type;

enum nubia_gpio_status {
 NUBIA_GPIO_PULL_DOWN = 0,//gpio pull down
 NUBIA_GPIO_FLOAT,//gpio float
 NUBIA_GPIO_PULL_UP,//gpio pull up
 NUBIA_GPIO_UNKNOWN,
};

struct hw_pcb_gpio_map_str{
	int gpio_A;
	int gpio_B;
	hw_pcb_type pcb_type;
	char pcb_ver[PCB_VER_LENGTH];
};

struct hw_rf_band_gpio_map_str{
	int gpio_A;
	int gpio_B;
	char rf_band[RF_BAND_LENGTH];
};

struct hw_config_gpio_map_st{
	int gpio_A;
	int gpio_B;
	char wifi_type[WIFI_STRING_LENGTH];
	char config_type[CONFIG_STRING_LENGTH];
};

/*HW_PCB_VESION 1.1*/
static const struct hw_pcb_gpio_map_str hw_pcb_gpio_map[] = {
	{NUBIA_GPIO_PULL_DOWN, NUBIA_GPIO_PULL_DOWN,	HW_A,  "MB_A"},
	{NUBIA_GPIO_PULL_DOWN, NUBIA_GPIO_PULL_UP,	    HW_B,  "MB_B"},
	{NUBIA_GPIO_PULL_UP,   NUBIA_GPIO_PULL_DOWN,    HW_C,  "MB_C"},
	{NUBIA_GPIO_PULL_UP,   NUBIA_GPIO_PULL_UP,      HW_D,  "MB_D"},
	{NUBIA_GPIO_FLOAT,     NUBIA_GPIO_PULL_UP,      HW_B1, "MB_B1"},
	{NUBIA_GPIO_PULL_UP,   NUBIA_GPIO_FLOAT,        HW_B2, "MB_B2"},
	{NUBIA_GPIO_FLOAT,     NUBIA_GPIO_PULL_DOWN,    HW_B3, "MB_B3"},
	{NUBIA_GPIO_PULL_DOWN, NUBIA_GPIO_FLOAT,        HW_C1, "MB_C1"},
	{NUBIA_GPIO_FLOAT,     NUBIA_GPIO_FLOAT,		HW_E,  "MB_E"},
};

//read rf band
static const struct hw_rf_band_gpio_map_str hw_rf_band_gpio_map[] = {
	{NUBIA_GPIO_PULL_DOWN, NUBIA_GPIO_PULL_DOWN,  "COMMON"},
	{NUBIA_GPIO_PULL_DOWN, NUBIA_GPIO_PULL_UP,    "CHINA"},
	{NUBIA_GPIO_PULL_UP,   NUBIA_GPIO_PULL_DOWN,  "ASIA_EUROPE"},
	{NUBIA_GPIO_PULL_UP,   NUBIA_GPIO_PULL_UP,    "AMERICAS"},
	{NUBIA_GPIO_PULL_DOWN, NUBIA_GPIO_FLOAT,      "THAILAND"},
	{NUBIA_GPIO_PULL_UP,   NUBIA_GPIO_FLOAT,      "JAPAN"},
};

//read config issue
static const struct hw_config_gpio_map_st hw_config_gpio_map[] = {
	{NUBIA_GPIO_PULL_DOWN, NUBIA_GPIO_PULL_DOWN,    "wifi_sky", "config"},
	{NUBIA_GPIO_FLOAT, NUBIA_GPIO_FLOAT,    "wifi_murata", "config"},
	{NUBIA_GPIO_PULL_DOWN, NUBIA_GPIO_PULL_UP,	"wifi_qorvo", "config"},

};
#endif
