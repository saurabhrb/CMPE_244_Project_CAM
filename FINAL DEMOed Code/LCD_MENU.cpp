/*
 * LCD_MENU.cpp
 *
 *  Created on: Apr 26, 2018
 *      Author: GyR0
 */

#include "LCD_MENU.hpp"

bool LCD_MENU::stream;

void LCD_MENU::highLight0()
{
    highlighted = 0;
}
LCD_MENU::LCD_MENU(LabGPIO_X CS, LabGPIO_X RST, unsigned int max_clock_mhz, str nm, LCD_MENU* CurrentMenu_, LCD_MENU* ParentMenu_, type mType)
{
    lcd = new LCD(CS, RST, 12);
    myName = nm;
    this->ParentMenu = ParentMenu_;
    this->CurrentMenu = CurrentMenu_;
    highlighted = 0;
    myType = mType;
    res = _352x288;
    stream = false;
}

LCD_MENU::LCD_MENU(LCD *lcd, str nm, LCD_MENU* CurrentMenu_, LCD_MENU* ParentMenu_, type mType)
{
    this->lcd = lcd;
    myName = nm;
    this->ParentMenu = ParentMenu_;
    this->CurrentMenu = CurrentMenu_;
    highlighted = 0;
    myType = mType;
}

LCD_MENU::LCD_MENU()
{
    highlighted = 0;
    myType = menuType;
    this->ParentMenu = nullptr;
    this->CurrentMenu = nullptr;
    myName = "NULL";
}

LCD_MENU::~LCD_MENU()
{

}

bool LCD_MENU::isELEPresent(str nm, int &index)
{
    for (uint8_t i = 0; i < ((uint8_t) all_elements_name.size()); i++) {
        if (all_elements_name[i] == nm) {
            index = i;
            return true;
        }
    }
    return false;

}

bool LCD_MENU::getSubMenu(str nm, LCD_MENU* SubMenu)
{
    int ind = -1;
    if (isELEPresent(nm, ind)) {
        SubMenu = SubMenus[ind];
        return true;
    }

    return false;
}

bool LCD_MENU::getItem(str nm, LCD_MENU* SubItem)
{
    int ind = -1;
    if (isELEPresent(nm, ind)) {
        SubItem = SubMenus[ind];
        return true;
    }

    return false;
}

LCD_MENU* LCD_MENU::confirmedGetSubMenu(str nm)
{
    int ind = -1;
    isELEPresent(nm, ind);
    return SubMenus[ind];
}

LCD_MENU* LCD_MENU::confirmedGetItem(str nm)
{
    int ind = -1;
    isELEPresent(nm, ind);
    return SubMenus[ind];
}

void LCD_MENU::dispallelements()
{
    u0_dbg_printf("\n\n== %s Elements==\n\n", myName.c_str());
    u0_dbg_printf("highlighted = %d\n", highlighted);

    for (uint8_t i = 0; i < ((uint8_t) all_elements.size()); i++) {
        u0_dbg_printf("%c%d: %s\n", ((all_elements[i].type == itemType) ? ' ' : '+'), all_elements[i].id, all_elements[i].name.c_str());
    }
    u0_dbg_printf("\n\n");

}

void LCD_MENU::scrollDown()
{
//    highlighted = (highlighted + 1) % all_elements.size();
    highlighted++;
    if (highlighted >= all_elements.size()) highlighted = 0;
}

void LCD_MENU::scrollUp()
{
    highlighted--;
    if (highlighted < 0) highlighted = all_elements.size() - 1;
}

LCD_MENU* LCD_MENU::loadNext()
{
    LCD_MENU *sub = this->SubMenus[highlighted];

//    if (stream) {
    stream = false;
    lcd->graphicsMode();
    lcd->fillRect(0, 0, 600, 400, RA8875_BLACK);
    lcd->textMode();
//    }

    if (sub->myType == itemType) {
        if (sub->myName.compareTo("160x120")) {
            res = _160x120;
            stream = true;
        }
        else if (sub->myName.compareTo("176x144")) {
            res = _176x144;
            stream = true;
        }
        else if (sub->myName.compareTo("320x240")) {
            res = _320x240;
            stream = true;
        }
        else if (sub->myName.compareTo("352x288")) {
            res = _352x288;
            stream = true;
        }
        else {
            res = _352x288;
        }

        str buff = "EXECUTED ITEM : ";

        buff.append(sub->myName);
        buff.append("()");
        lcd->textEnlarge(2);
        lcd->textColor(RA8875_DARK_ORANGE, RA8875_BLACK);
        lcd->textSetCursor(200 - 50, 300);
        lcd->textWrite(buff.c_str(), buff.getLen());
//                u0_dbg_printf("\n\nEXECUTED ITEM : %s()\n\n", SubMenus[highlighted]->myName.c_str());
    }
    else {
        if (sub->all_elements.size() == 0) {
            str buff = "SUB-MENU EMPTY!";
            lcd->textEnlarge(2);
            lcd->textColor(RA8875_DARK_ORANGE, RA8875_BLACK);
            lcd->textSetCursor(200, 300);
            lcd->textWrite(buff.c_str(), buff.getLen());
//                u0_dbg_printf("\n\nSUB-MENU EMPTY\n\n");
        }
        else {
            sub->highLight0();
            stream = false;
            return this->SubMenus[highlighted];
        }
    }
    return this;
}

LCD_MENU* LCD_MENU::loadPrev()
{

    if (ParentMenu == nullptr) {
        str buff = "ALREADY IN MAIN MENU!";
        lcd->textEnlarge(2);
        lcd->textColor(RA8875_DARK_ORANGE, RA8875_BLACK);
        lcd->textSetCursor(200, 300);
        lcd->textWrite(buff.c_str(), buff.getLen());
//                u0_dbg_printf("\n\nALREADY IN MAIN MENU\n\n");
    }
    else {
        lcd->graphicsMode();
        lcd->fillRect(1, 1, 600, 400, RA8875_BLACK);
        lcd->textMode();
        stream = false;
        return ParentMenu;
    }
    return this;
}

//loads the image
void LCD_MENU::clear_disp(uint16_t photo[])
{
    lcd->graphicsMode();
    lcd->fillScreen(RA8875_BLACK);
    int k = 0;
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            uint16_t colr = photo[k++];
            lcd->drawPixel(400 + i - 100, 240 + j - 100, colr);
        }
    }
}

void LCD_MENU::Display()
{
    int main_scale = 1;
    int sub_scale = 1;

    int main_width = (main_scale + 1) * (8);
    int main_height = (main_scale + 1) * (16);

    int sub_width = (sub_scale + 1) * (8);
    int sub_height = (sub_scale + 1) * (16);

    //clear display
    //lcd->graphicsMode();
    //lcd->fillScreen(RA8875_BLACK);

    lcd->graphicsMode();
    lcd->fillRect(600 - 2, 0, 800 - 1 - 600 + 2, 480 - 1, RA8875_BLACK);
    lcd->fillRect(0, 300, 600 - 3, 480 - 1 - 300, RA8875_BLACK);
    lcd->drawRect(600 - 2, 0, 200 + 1, 480 - 1, RA8875_WHITE);

    //render display
    LCD_MENU *p = ParentMenu;
    str path = this->myName;
    while (p) {
        path.insertAtBeg("->");
        path.insertAtBeg(p->myName);
        p = p->ParentMenu;
    }

    int x = 600;
    int menu_width = 800 - x;
    int y = 10;

    lcd->textMode();
//    vTaskDelay(150);
    lcd->textColor(RA8875_WHITE, RA8875_BLACK);
    lcd->textSetCursor(x, y);
    lcd->textEnlarge(main_scale);

    str rem = path;
    bool small = false;
    while (rem.getLen() > menu_width / main_width) {
        lcd->textWrite(rem.c_str(), menu_width / main_width);
        rem = path.subString(menu_width / main_width, path.getLen() + 1 - (menu_width / main_width));
        y = lcd->getCurrY() + main_height + 10;
        lcd->textSetCursor(x, y);
        small = true;
    }
    if (rem.getLen() < menu_width / main_width) lcd->textWrite(rem.c_str(), rem.getLen());

    y = lcd->getCurrY() + main_height + 50;

    for (uint8_t i = 0; i < ((uint8_t) all_elements.size()); i++) {
        lcd->textColor(RA8875_WHITE, RA8875_BLACK);
        str buff = "";
        if (highlighted == i) lcd->textColor(RA8875_BLACK, RA8875_WHITE);

        str test;
        test.printf("%d:%s", all_elements[i].id, all_elements[i].name.c_str());
        buff.append(test);

        if (all_elements[i].type == itemType) buff.append(" ");
        else
            buff.append(" +");

        lcd->textSetCursor(x, y);
        lcd->textEnlarge(sub_scale);

        str rem = buff;
        bool small = false;
        while (rem.getLen() > menu_width / sub_width) {
            lcd->textWrite(rem.c_str(), menu_width / sub_width);
            rem = buff.subString(menu_width / sub_width, buff.getLen() + 1 - (menu_width / sub_width));
            y = lcd->getCurrY() + sub_height + 10;
            lcd->textSetCursor(x, y);
            small = true;
        }
        if (rem.getLen() < menu_width / sub_width) lcd->textWrite(rem.c_str(), rem.getLen());
        y = lcd->getCurrY() + sub_height + 10;
    }

    //matches
    if (this->myName.compareTo("About")) {
        lcd->textColor(RA8875_WHITE, RA8875_BLACK);
        x = 1;
        y = 1;
        menu_width = 600 - x;
        lcd->textSetCursor(x, y);
        lcd->textEnlarge(main_scale);

        path = "Team NEMESIS:\n\nWe are a group of 5 who have implemented a digital camera using an SPI LCD and an SPI Camera";
        rem = path;
        small = false;
        while (rem.getLen() > (menu_width) / main_width) {
            lcd->textWrite(rem.c_str(), menu_width / main_width);
            rem = path.subString(menu_width / main_width, path.getLen() + 1 - (menu_width / main_width));
            y = lcd->getCurrY() + main_height + 10;
            lcd->textSetCursor(x, y);
            small = true;
        }
        if (rem.getLen() < menu_width / main_width) lcd->textWrite(rem.c_str(), rem.getLen());

    }
    else if (this->myName.compareTo("Members")) {
           lcd->textColor(RA8875_WHITE, RA8875_BLACK);
           x = 1;
           y = 1;
           menu_width = 600-x;
           lcd->textSetCursor(x, y);
           lcd->textEnlarge(main_scale);

           path = "Components:\n\nLCD:\n5.0\" TFT Screen with 800x480 pixels\nCamera:\nOV2640 SPI Arducam with Buffer";
           rem = path;
           small = false;
           while (rem.getLen() > (menu_width) / main_width) {
               lcd->textWrite(rem.c_str(), menu_width / main_width);
               rem = path.subString(menu_width / main_width, path.getLen() + 1 - (menu_width / main_width));
               y = lcd->getCurrY() + main_height + 10;
               lcd->textSetCursor(x, y);
               small = true;
           }
           if (rem.getLen() < menu_width / main_width) lcd->textWrite(rem.c_str(), rem.getLen());

       }

}
