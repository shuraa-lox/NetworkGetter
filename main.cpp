#include <iostream>
#include "pugixml.hpp"
#include <sstream>
#include <windows.h>
#include <clocale>
#include <codecvt> 
#include <locale>  
#include <filesystem> // Нужен C++17

namespace fs = std::filesystem;
using namespace std;

wstring to_wstring(const std::string& str) {
    // Создаем конвертер из UTF-8 в UTF-16 (wchar_t)
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

int main(){
	setlocale(LC_ALL, ""); // Кодировка

	stringstream ss; // Создаю "редактор строки"
	wstringstream wss; // Создаю "редактор wстроки"
	string name;

	cout << "Print network name: ";
	cin >> name;

	fs::create_directory("networks"); // Создаю папку, в которой будут сети
	fs::path current_path = fs::current_path() / "networks"; //Получаю путь к директории, в которой лежит программа а затем добавляю к ней папку

	ss << "netsh wlan export profile name=\"" << name << "\" folder=" << current_path << " key=clear > nul"; //Совмещаю все в бульйон
	string stroke = ss.str(); //Бульйон => строка
	const char* cmd = stroke.c_str(); //Строка => массив чаров(систем принимает массив чаров а не строку)
	system(cmd); // Выполняем команду

	try {
		pugi::xml_document doc; // Создаю обьект класса либы, которая читает хмл файлы
        for (const auto& entry : fs::directory_iterator(current_path)) {
        	wstring filepath = entry.path().wstring(); // wСтрока(потому что ютф8 для кирилицы и пробелов) в которой лежит путь к созданому файлу
			wstring w_name = to_wstring(name); // Делаю имя wстрокой чтобы совместить с путем без конфликта типов
		
        	wss << w_name << L".xml"; // Совмещаю все в бульйон 
		
        	wstring fullname = wss.str(); // Перевожу бульйон в wстроку
        	wstring newpath = current_path/fullname; // Новый путь с переименованой сетью

        	fs::rename(filepath, newpath); // Переименовываю кирилицу и пробелы в обычное имя сети, чтобы либа нормально нашла его
	
			doc.load_file(newpath.c_str()); // Подгружаю переименованый файл в либу
   			pugi::xml_node keyMaterial = doc.child("WLANProfile").child("MSM").child("security").child("sharedKey").child("keyMaterial"); // Достаю нужный тэг
   			string password = keyMaterial.child_value(); // Полчаю пароль с нужного тега
		
   			if (password != ""){
   				cout << "Password: " << password << endl;
   			}else{
   				cout << "There is no password on the \"" << name << "\" network." << endl;
   			}

   			system("pause");
   		}
   		
    } catch (const fs::filesystem_error& e) {
        cout << "Ошибка: " << e.what() << endl;
    }

    fs::remove_all(current_path); // Удаляю папку с сетями

	return 0;
}
