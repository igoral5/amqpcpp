#include "AMQPcpp.h"
int i=0;

int onCancel(AMQPMessage * message ) {
	std::cout << "cancel tag="<< message->getDeliveryTag() << std::endl;
	return 0;
}

int  onMessage( AMQPMessage * message  ) {
	uint32_t j = 0;
	std::cout << message->getMessage(&j) << std::endl;

	i++;

	std::cout << "#" << i << " tag="<< message->getDeliveryTag() << " content-type:"<< message->getHeader("Content-type") ;
	std::cout << " encoding:"<< message->getHeader("Content-encoding")<< " mode="<<message->getHeader("Delivery-mode")<< std::endl;

	return 0;
};


int main () {


	try {
//		AMQP amqp("123123:akalend@localhost/private");

		AMQP amqp("guest:guest@pluto-v302.search.km:5672");

		AMQPQueue * qu2 = amqp.createQueue("hello");

		qu2->Declare();

		qu2->addEvent(AMQP_MESSAGE, onMessage );
		qu2->addEvent(AMQP_CANCEL, onCancel );

		qu2->Consume(AMQP_NOACK);//


	} catch (AMQPException e) {
		std::cout << e.getMessage() << std::endl;
	}

	return 0;

}

