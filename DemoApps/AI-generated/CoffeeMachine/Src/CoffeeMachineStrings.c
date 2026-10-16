#include "CoffeeMachineStrings.h"

static const char *const s_strings[2][COFFEE_STR_COUNT] = {
    {
        "Ready", "Busy", "Error", "Cleaning", "Hot water",
        "Water", "Beans", "Grounds",
        "Tap a drink to start", "Standby", "Touch anywhere to wake",
        "Strength", "Size", "Milk ratio", "Temperature", "Warm", "Hot",
        "Extra hot", "Back", "Start", "Stop", "Grinding", "Brewing",
        "Milk", "Done", "Language / 语言", "English", "Simplified Chinese",
        "Controller heartbeat lost"
    },
    {
        "就绪", "忙碌", "故障", "清洁", "热水",
        "水量", "咖啡豆", "咖啡渣",
        "轻触饮品开始", "待机", "轻触屏幕唤醒",
        "浓度", "容量", "奶量", "温度", "温", "热",
        "烫", "返回", "开始", "停止", "研磨", "萃取",
        "打奶", "完成", "Language / 语言", "English", "简体中文",
        "控制器通信中断"
    }
};

static const char *const s_drinks[2][COFFEE_DRINK_COUNT] = {
    {
        "Espresso", "Lungo", "Americano", "Cappuccino",
        "Latte", "Flat White", "Hot Water", "Milk Foam"
    },
    {
        "意式浓缩", "长萃咖啡", "美式咖啡", "卡布奇诺",
        "拿铁", "澳白", "热水", "奶泡"
    }
};

const char *CoffeeStrings_Get(CoffeeLanguage language, CoffeeStringId id)
{
    if ((unsigned)language > COFFEE_LANGUAGE_CHINESE || (unsigned)id >= COFFEE_STR_COUNT)
        return "";
    return s_strings[language][id];
}

const char *CoffeeStrings_Drink(CoffeeLanguage language, CoffeeDrinkId drink)
{
    if ((unsigned)language > COFFEE_LANGUAGE_CHINESE || (unsigned)drink >= COFFEE_DRINK_COUNT)
        return "";
    return s_drinks[language][drink];
}
