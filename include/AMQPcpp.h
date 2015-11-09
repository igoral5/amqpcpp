/*
 *  amqpcpp.h
 *  librabbitmq++
 *
 *  Created by Alexandre Kalendarev on 01.03.10.
 *
 */

#ifndef __AMQPCPP
#define __AMQPCPP

#define AMQPPORT 5672
#define AMQPHOST "localhost"
#define AMQPVHOST "/"
#define AMQPLOGIN "guest"
#define AMQPPSWD  "guest"

#define AMQPDEBUG ":5673"

#define AMQP_AUTODELETE		1
#define AMQP_DURABLE		2
#define AMQP_PASSIVE		4
#define AMQP_MANDATORY		8
#define AMQP_IMMIDIATE		16
#define AMQP_IFUNUSED		32
#define AMQP_EXCLUSIVE		64
#define AMQP_NOWAIT			128
#define AMQP_NOACK			256
#define AMQP_NOLOCAL		512
#define AMQP_MULTIPLE		1024
#define AMQP_INTERNAL       2048


#define HEADER_FOOTER_SIZE 8 //  7 bytes up front, then payload, then 1 byte footer
#define FRAME_MAX 131072    // max lenght (size) of frame

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "strings.h"

#include <unistd.h>
#include <stdint.h>

#include "amqp.h"
#include "amqp_framing.h"
#include "amqp_tcp_socket.h"

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <sstream>

//export AMQP;
//using namespace std;

class AMQPQueue;

enum AMQPEvents_e {
	AMQP_MESSAGE, AMQP_SIGUSR, AMQP_CANCEL, AMQP_CLOSE_CHANNEL
};

class AMQPException {
	std::string message;
	int code;
	public:
		AMQPException(const std::string& message);
		AMQPException(amqp_rpc_reply_t * res);

		std::string   getMessage();
		uint16_t getReplyCode();
};



class AMQPMessage {

	char * data;
	uint32_t len;
	std::string exchange;
	std::string routing_key;
	uint32_t delivery_tag;
	int message_count;
	std::string consumer_tag;
	AMQPQueue * queue;
	std::map<std::string, std::string> headers;

	public :
		AMQPMessage(AMQPQueue * queue);
		~AMQPMessage();

		void setMessage(const char * data,uint32_t length);
		std::string getMessage(uint32_t* length);

		void addHeader(const std::string& name, amqp_bytes_t * value);
		void addHeader(const std::string& name, uint64_t * value);
		void addHeader(const std::string& name, uint8_t * value);
		void addHeader(amqp_bytes_t * name, amqp_bytes_t * value);
		std::string getHeader(const std::string& name);

		void setConsumerTag( amqp_bytes_t consumer_tag);
		void setConsumerTag( const std::string& consumer_tag);
		std::string getConsumerTag();

		void setMessageCount(int count);
		int getMessageCount();

		void setExchange(amqp_bytes_t exchange);
		void setExchange(const std::string& exchange);
		std::string getExchange();

		void setRoutingKey(amqp_bytes_t routing_key);
		void setRoutingKey(const std::string& routing_key);
		std::string getRoutingKey();

		uint32_t getDeliveryTag();
		void setDeliveryTag(uint32_t delivery_tag);

		AMQPQueue * getQueue();
};


class AMQPBase {
	protected:
		std::string name;
		short parms;
		amqp_connection_state_t * cnn;
		int channelNum;
		AMQPMessage * pmessage;

		short opened;

		void checkReply(amqp_rpc_reply_t * res);
		void checkClosed(amqp_rpc_reply_t * res);
		void openChannel();


	public:
		virtual ~AMQPBase();
		int getChannelNum();
		void setParam(short param);
		std::string getName();
		void closeChannel();
		void reopen();
		void setName(const char * name);
		void setName(const std::string& name);
};

class AMQPQueue : public AMQPBase  {
	protected:
		std::map< AMQPEvents_e, int(*)( AMQPMessage * ) > events;
		amqp_bytes_t consumer_tag;
		uint32_t delivery_tag;
		uint32_t count;
	public:
		AMQPQueue(amqp_connection_state_t * cnn, int channelNum);
		AMQPQueue(amqp_connection_state_t * cnn, int channelNum, const std::string& name);

		void Declare();
		void Declare(const std::string& name);
		void Declare(const std::string& name, short parms);

		void Delete();
		void Delete(const std::string& name);

		void Purge();
		void Purge(const std::string& name);

		void Bind(const std::string& exchangeName, const std::string& key);

		void unBind(const std::string& exchangeName, const std::string& key);

		void Get();
		void Get(short param);

		void Consume();
		void Consume(short param);

		void Cancel(amqp_bytes_t consumer_tag);
		void Cancel(const std::string& consumer_tag);

		void Ack();
		void Ack(uint32_t delivery_tag);

		AMQPMessage * getMessage() {
			return pmessage;
		}

		uint32_t getCount() {
			return count;
		}

		void setConsumerTag(const std::string& consumer_tag);
		amqp_bytes_t getConsumerTag();

		void addEvent( AMQPEvents_e eventType, int (*event)(AMQPMessage*) );

		virtual ~AMQPQueue();
		
		void Qos(uint32_t prefetch_size, uint16_t prefetch_count, amqp_boolean_t global );
	private:
		void sendDeclareCommand();
		void sendDeleteCommand();
		void sendPurgeCommand();
		void sendBindCommand(const char * exchange, const char * key);
		void sendUnBindCommand(const char * exchange, const char * key);
		void sendGetCommand();
		void sendConsumeCommand();
		void sendCancelCommand();
		void sendAckCommand();
		void setHeaders(amqp_basic_properties_t * p);
};


class AMQPExchange : public AMQPBase {
	std::string type;
	std::map<std::string, std::string> sHeaders;
	std::map<std::string, std::string> sHeadersSpecial;
	std::map<std::string, int> iHeaders;

	public:
		AMQPExchange(amqp_connection_state_t * cnn, int channelNum);
		AMQPExchange(amqp_connection_state_t * cnn, int channelNum, const std::string& name);
		virtual ~AMQPExchange();

		void Declare();
		void Declare(const std::string& name);
		void Declare(const std::string& name, const std::string& type);
		void Declare(const std::string& name, const std::string& type, short parms);

		void Delete();
		void Delete(const std::string& name);

		void Bind(const std::string& queueName);
		void Bind(const std::string& queueName, const std::string& key);

		void Publish(const std::string& message, const std::string& key);
		void Publish(const char * data, uint32_t length, const std::string& key);

		void setHeader(const std::string& name, int value);
		void setHeader(const std::string& name, const std::string& value);
		void setHeader(const std::string& name, const std::string& value, bool special);

	private:
		AMQPExchange();
		void checkType();
		void sendDeclareCommand();
		void sendDeleteCommand();
		void sendPublishCommand();

		void sendBindCommand(const char * queueName, const char * key);
		void sendPublishCommand(amqp_bytes_t messageByte, const char * key);
		void sendCommand();
		void checkReply(amqp_rpc_reply_t * res);
		void checkClosed(amqp_rpc_reply_t * res);

};

class AMQP {
	int port;
	std::string host;
	std::string vhost;
	std::string user;
	std::string password;
	amqp_socket_t *sockfd;
	int channelNumber;

	amqp_connection_state_t cnn;
	AMQPExchange * exchange;

	std::vector<AMQPBase*> channels;

	public:
		AMQP();
		explicit AMQP(const std::string& cnnStr);
		virtual ~AMQP() noexcept;

		AMQPExchange * createExchange();
		AMQPExchange * createExchange(const std::string& name);

		AMQPQueue * createQueue();
		AMQPQueue * createQueue(const std::string& name);

		void printConnect();

		void closeChannel();

	private:
		//AMQP& operator =(AMQP &ob);
		AMQP( AMQP &ob );
		void init();
		void initDefault();
		void connect();
		void parseCnnString(const std::string& cnnString );
		void parseHostPort(const std::string& hostPortString );
		void parseUserStr(const std::string& userString );
		void sockConnect();
		void login();
		//void chanalConnect();
};

#endif //__AMQPCPP
