#include "forwarding.h"




forwarding::forwarding(routerMain* m)
{
	route_table = &(m->route_table);
	nei_msg_mtx = &(m->nei_msg_mtx);
	lsa_msg_mtx = &(m->lsa_msg_mtx);
	my_msg_mtx = &(m->for_msg_lsa_mtx);

	nei_msg_q = &(m->msgq_nei);
	lsa_msg_q = &(m->msgq_lsa);
	my_msg_q = &(m->msgq_for_lsa);

	my_id = &(m->router_id);
	port = &(m->port);
}




//setting flag, wait some time and exit
forwarding::~forwarding()
{
}


//connect to controller to get id, blocking method
void forwarding::initialize()
{

	*my_id = 1;//place holder for testing
	id_ready = true;
	return;
}

//maintain message queues and forward messages
void forwarding::run(void* __this)
{
	forwarding* _this = (forwarding*)__this;
	thread tcp_server_t(forwarding::tcp_server, _this);
	tcp_server_t.detach();


	while (_this->running_flag)
	{

	}

}

//used to accept tcp connection, start in run
void forwarding::tcp_server(void* __this)
{
	forwarding* _this = (forwarding*)__this;

	tcp::acceptor a(_this->io_context, tcp::endpoint(tcp::v4(), *_this->port));
	for (;;)
	{
		std::thread(forwarding::tcp_session, _this, a.accept()).detach();
		this_thread::sleep_for(chrono::milliseconds(READ_INTERVAL));
	}
}

//accepting messages. dispatcher
void forwarding::tcp_session(void *__this, tcp::socket sock)
{
	forwarding* _this = (forwarding*)__this;

	try
	{
		for (;;)
		{
			char data[BUFFER_LENGTH];

			boost::system::error_code error;
			size_t length = sock.read_some(boost::asio::buffer(data), error);
			if (error == boost::asio::error::eof)
				break; // Connection closed cleanly by peer.
			else if (error)
				throw boost::system::system_error(error); // Some other error.


			boost::asio::write(sock, boost::asio::buffer(data, length));

			this_thread::sleep_for(chrono::milliseconds(READ_INTERVAL));
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception in thread: " << e.what() << "\n";
	}
}
