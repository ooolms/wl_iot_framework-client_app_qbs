/*******************************************
Copyright 2017 OOO "LMS"

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include "wliot/client/ServerInstance.h"
#include "wliot/client/VirtualDeviceClient.h"
#include "wliot/client/VirtualDeviceCallback.h"
#include "CmdArgParser.h"

using namespace WLIOT;
using namespace WLIOTClient;

/*
 * В данном примере производится подключение к локальному серверу, затем регистрируется виртуальное устройство,
 * передающее по таймеру значения с неких датчиков температуры и влажности (на самом деле рандомные значения) и
 * выполняющее команду мигания неким светодиодом (выводит в консоль "Blink command" при поступлении команды).
*/

QUuid deviceId=QUuid("{3a4b236c-1da6-4332-9c43-86c83bfbae4e}");//идентификатор виртуального устройства

QByteArray deviceName="TestDev";//название виртуального устройства

VirtualDeviceClient *vDev=0;

//функция, генерирующая список датчиков
QList<SensorDef> mkSensors()
{
	QList<SensorDef> retVal;
	//датчик температуры, единичные (SINGLE) одномерные (1) вещественные значения обычной точности (F32)
	//без метки времени (NO_TIME)
	retVal.append(SensorDef(SensorDef::Type(SensorDef::F32,SensorDef::SINGLE,SensorDef::NO_TIME,1),"temp"));
	//датчик влажности, аналогично температурному
	retVal.append(SensorDef(SensorDef::Type(SensorDef::F32,SensorDef::SINGLE,SensorDef::NO_TIME,1),"hum"));
	return retVal;
}

//функция, генерирующая описание элементов управления (команд)
ControlsGroup mkControls()
{
	//главная группа команд
	ControlsGroup grp;
	//команда
	ControlsCommand cmd;
	//текст на кнопке
	cmd.buttonText="blink";
	//команда, передаваемая устройству
	cmd.commandToExec="blink";
	//заголовок команды, показывается, если в команду добавить несколько параметров
	cmd.title="Blink";
	//добавляем команду в группу
	grp.elements.append(ControlsGroup::Element(cmd));
	return grp;
}

//функция отправки значений, вызываемая по таймеру
void onTimer()
{
	//отправляем температуру
	vDev->sendVDevMeasurement("temp",QByteArrayList()<<QByteArray::number(20.0+(qrand()%10)/10.0));
	//отправляем влажность
	vDev->sendVDevMeasurement("hum",QByteArrayList()<<QByteArray::number(35.0+(qrand()%10)));
}

//callback-класс, обрабатывающий команды устройству, приходящие от сервера
class VDevCallback
	:public VirtualDeviceCallback
{
public:
	/**
	 * @brief processCommand функция, обрабатывающая команды устройству
	 * @param cmd команда
	 * @param args аргументы команды
	 * @param retVal возвращаемые значения, которые будут переданы серверу
	 * @return
	 */
	virtual bool processCommand(const QByteArray &cmd,const QByteArrayList &args,QByteArrayList &retVal)override
	{
		Q_UNUSED(args)
		Q_UNUSED(retVal)
		if(cmd=="blink")
		{
			//команда blink
			qDebug()<<"Blink command";
			return true;
		}
		else
		{
			//неизвестная команда
			retVal.append("unknown command");
			return false;
		}
	}
};

int main(int argc,char *argv[])
{
	QCoreApplication app(argc,argv);
	//parser - объект для паркинга аргуметов командной строки
	CmdArgParser parser(app.arguments());
	//хост локального сервера - доменное имя или ip-адрес
	QString host=parser.getVarSingle("host");
	//пользователь для авторизации на локальном сервере
	QByteArray user=parser.getVarSingle("user").toUtf8();
	//пароль для авторизации на локальном сервере
	QByteArray pass=parser.getVarSingle("pass").toUtf8();
	//порт локального сервера, лучше не задавать, так как на данный момент
	//сервер можно запустить только на стандартном порту 4083
	quint16 port=WLIOTServerProtocolDefs::controlSslPort;
	if(!parser.getVarSingle("port").isEmpty())
		port=parser.getVarSingle("port").toUShort();
	if(port==0)
		port=WLIOTServerProtocolDefs::controlSslPort;
	bool netMode=!host.isEmpty()&&!user.isEmpty();

	//создаем объект ServerInstance
	ServerInstance srv;
	//устанавливаем данные для авторизации
	srv.connection()->prepareAuth(user,pass);

	//подключаемся к серверу
	if(!netMode)
		srv.connection()->startConnectLocal();
	else srv.connection()->startConnectNet(host,port);
	if(!srv.connection()->waitForConnected())
		return __LINE__;
	if(!srv.connection()->isReady())
		return __LINE__;

	//регистрируем виртуальное устройство
	VDevCallback cb;
	vDev=srv.devices()->registerVirtualDevice(deviceId,deviceName,mkSensors(),mkControls(),QUuid(),&cb);
	if(!vDev)
		return __LINE__;

	//завершаем работу в случае обрыва соединения с сервером
	QObject::connect(srv.connection(),&ServerConnection::disconnected,qApp,&QCoreApplication::quit);

	//устанавливаем и запускаем таймер для отправки значений датчика
	QTimer timer;
	timer.setInterval(10000);
	timer.setSingleShot(false);
	QObject::connect(&timer,&QTimer::timeout,&onTimer);
	timer.start();

	int r=app.exec();
	timer.stop();
	return r;
}

