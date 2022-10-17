/*
* 옵저버 패턴
* 1:N관계를 정의할때 사용
* 1 = 옵저버
* N = 주제
*/

/*
#include <iostream>
#include <string>
#include <map>
class Observer { // 인터페이스 클래스
public:
	virtual void Update(float fData1, float fData2, float fData3) = 0;
	std::string _szName;
};

class Subject { // 인터페이스 클래스
public:
	virtual void RegisterObserver(Observer *pO) = 0;
	virtual void RemoveObserver(Observer *pO) = 0;
	virtual void NotifyObservers(void) = 0;
};

class WeatherData : public Subject { // 실제 주제 데이터 객체
private:
	float _fData1;
	float _fData2;
	float _fData3;

private :
	typedef std::map<std::string, Observer *>	STL_MAP_OBSERBER;
	typedef STL_MAP_OBSERBER::iterator			STL_MAP_OBSERBER_ITR;
	typedef STL_MAP_OBSERBER::value_type		STL_MAP_OBSERBER_VT;
	STL_MAP_OBSERBER							_map_observer;
public:
	float GetData1() { return _fData1; };
	float GetData2() { return _fData2; };
	float GetData3() { return _fData3; };
	void SetData(float fData1, float fData2, float fData3) {
		_fData1 = fData1;
		_fData2 = fData2;
		_fData3 = fData3;
		NotifyObservers();
	}
public:
	virtual void RegisterObserver(Observer *pO) {
		_map_observer.insert(STL_MAP_OBSERBER_VT(pO->_szName, pO));
	}
	virtual void RemoveObserver(Observer *pO) {
		_map_observer.erase(pO->_szName);
	}
	virtual void NotifyObservers(void) {
		STL_MAP_OBSERBER_ITR itr = _map_observer.begin();
		while (itr != _map_observer.end()) {
			Observer *pObserver = itr->second;
			pObserver->Update(_fData1, _fData2, _fData3);
			++itr;
		}
	}
};

class ConditionDisplay : public Observer {
private:
	Subject *_pSubject;
	float _fData1;
	float _fData2;
	float _fData3;
public:
	ConditionDisplay(Subject *pSubject) {
		pSubject->RegisterObserver(this);
		_pSubject = pSubject;
	}
public:
	void Display() {
		std::cout << "디스플레이 온도 : " << _fData1 <<std::endl;
		std::cout << "디스플레이 습도 : " << _fData2 <<std::endl;
		std::cout << "디스플레이 입력 : " << _fData3 <<std::endl;
		std::cout <<std::endl;
	}
public:
	virtual void Update(float fData1, float fData2, float fData3) {
		_fData1 = fData1;
		_fData2 = fData2;
		_fData3 = fData3;
	}
};

int main() {
	WeatherData *pWeatherData = new WeatherData;
	ConditionDisplay *pConditionDisplay = new ConditionDisplay(pWeatherData);

	pWeatherData->SetData(10.5f, 20.5f, 30.5f);
	pConditionDisplay->Display();
}
*/