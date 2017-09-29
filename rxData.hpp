#include <iostream>
#include <string>
#include <list>
#include <map>

template<typename T>
class rxData  {
		static int counter;
		T value_;
		std::list<rxData*> subject;
		std::multimap<rxData*, std::function<void (T)> >
			subscriber;
		std::map<int, std::function<void (T)> >
			observer;
		bool isDyn;
		bool unFree;
		std::string op;
		int id;

	public:
		void	setOp(std::string op) { this->op = op; }

		int getId() {
			return (this->id);
		}
		template <typename U>
		rxData<U> &rxCast() {
			rxData<U> *rt = new rxData<U>;
			rt->setOp("rxCast");
			this->subscribe(rt, [rt](T data){
				rt->notify((U)data);
			});
			return (*rt);
		}

		template<typename U, typename V>
		static rxData		&merge2(rxData<U> &a, rxData<V> &b, std::function<T (U a, V b)> op) {
			rxData *rt =  rxData::create();
			rt->op = "merge2";
			a.subscribe(rt, [rt, op, &b](U a) {
				rt->notify(op(a, b.getValue()));
			});
			b.subscribe(rt, [rt, op, &a](V b) {
				rt->notify(op(a.getValue(), b));
			});
			rt->notify(op(a.getValue(), b.getValue()));
			return (*rt);
		}

		template<typename U, typename V, typename W>
		static rxData		&merge3(rxData<U> &a, rxData<V> &b, rxData<W> &c, std::function<T (U a, V b, W c)> op) {
			rxData *rt =  rxData::create();
			rt->op = "merge3";
			a.subscribe(rt, [rt, op, &b, &c](U a) {
				rt->notify(op(a, b.getValue(), c.getValue()));
			});
			b.subscribe(rt, [rt, op, &a, &c](V b) {
				rt->notify(op(a.getValue(), b, c.getValue()));
			});
			c.subscribe(rt, [rt, op, &a, &b](W c) {
				rt->notify(op(a.getValue(), b.getValue(), c));
			});
			rt->notify(op(a.getValue(), b.getValue(), c.getValue()));
			return (*rt);
		}
		
		template<typename U, typename V, typename W, typename X>
		static rxData		&merge4(rxData<U> &a, rxData<V> &b, rxData<W> &c, rxData<X> &d, std::function<T (U a, V b, W c, X d)> op) {
			rxData *rt =  rxData::create();
			rt->op = "merge4";
			a.subscribe(rt, [rt, op, &b, &c, &d](U a) {
				rt->notify(op(a, b.getValue(), c.getValue(), d.getValue()));
			});
			b.subscribe(rt, [rt, op, &a, &c, &d](V b) {
				rt->notify(op(a.getValue(), b, c.getValue(), d.getValue()));
			});
			c.subscribe(rt, [rt, op, &a, &b, &d](W c) {
				rt->notify(op(a.getValue(), b.getValue(), c, d.getValue()));
			});
			d.subscribe(rt, [rt, op, &a, &b, &c](X d) {
				rt->notify(op(a.getValue(), b.getValue(), c.getValue(), d));
			});
				rt->notify(op(a.getValue(), b.getValue(), c.getValue(), d.getValue()));
			return (*rt);
		}

		rxData() : isDyn(false), unFree(false), op(" ") {
			this->id = ++rxData::counter;
		}

		rxData(T value) : value_(value), isDyn(false), unFree(false), op(" ") { 
			this->id = ++counter;
		}

		~rxData() {
			this->resetSubject();
			for (auto it = this->subscriber.begin(); it != this->subscriber.end();it++)
				it->first->subject.remove(this);
		}

		static rxData<T> *create() {
			rxData *rt = new rxData;
			rt->isDyn = true;
			return (rt);
		}

		static rxData<T> *create(bool unFree) {
			rxData *rt = new rxData;
			rt->setUnFree(unFree);
			rt->isDyn = true;
			return (rt);
		}

		void setUnFree(bool unFree) { this->unFree = unFree; }

		friend std::ostream& operator <<(std::ostream& os, const rxData & toPrint) {
			os << toPrint.getValue();
			return os;
		}

		void printDebug(std::ostream& os) {
			os << this->id << "[" << this->getValue() << "] " << this->op;
		}

		
		T getValue() const { return (this->value_); };

		rxData &operator=(rxData & other) {
			this->resetSubject();
			other.subscribe(this, [this](T data) {
				this->notify(data);
			});
			this->notify(other.value_);
			return (*this);
		}

		rxData operator=(T data) {
			this->resetSubject();
			this->value_ = data;
			this->commit();
			return (*this);
		}

		void resetSubject() {
			for (auto it = this->subject.begin(); it != this->subject.end();it++)
			{
				(*it)->unsubscribe(this);
				this->subject.erase(it);
			}
		}

		operator T () {
			return (this->value_);
		}

		void	observe(int key, std::function<void (T)> command) {
			this->observer.insert(std::pair<int,std::function<void (T)> >(key, command));
			command(this->value_);
		}

		void subscribe(void *who, std::function<void (T)> command) {
			this->subscriber.insert(std::pair<rxData*,std::function<void (T)> >((rxData*)who, command));
				((rxData*)who)->subject.push_back(this);
		}

		rxData &map(std::function<T (T)> command) {
			rxData	*rt =  rxData::create();
			this->subscribe(rt, [rt, command](T data) {
				rt->notify(command(data));
			});
			rt->notify(command(this->value_));
			rt->op = "map";
			return (*rt);
		}

		void commit() {
			for(auto it = this->observer.begin(); it != this->observer.end(); it++)
				it->second(this->value_);
			for(auto it = this->subscriber.begin(); it != this->subscriber.end(); it++)
				it->second(this->value_);
		}

		rxData &doOp(rxData &other, std::function<T (T, T)> op) {
			rxData	*rt =  rxData::create();
			this->subscribe(rt, [rt, &other, op](T data) {
				T ndata = other.value_ + data;
				if (ndata != rt->getValue())
					rt->notify(op(data, other.value_));
			});
			other.subscribe(rt, [rt, this, op](T data) {
				T ndata = this->value_ + data;
				if (ndata != rt->getValue())
					rt->notify(op(this->value_, data));
			});
			rt->notify(op(this->value_, other.value_));
			return (*rt);
		}

		rxData &operator%(rxData & other) {
			rxData &rt =this->doOp(other, [](T a, T b) { return (a % b); });
			rt.op = "%";
			return (rt);
		}

		rxData &operator/(rxData & other) {
			rxData &rt = this->doOp(other, [](T a, T b) { return (a / b); });
			rt.op = "/";
			return (rt);
		}

		rxData &operator*(rxData & other) {
			rxData &rt = this->doOp(other, [](T a, T b) { return (a * b); });
			rt.op = "*";
			return (rt);
		}

		rxData &operator-(rxData & other) {
			rxData &rt = this->doOp(other, [](T a, T b) { return (a - b); });
			rt.op = "-";
			return (rt);
		}

		rxData &operator+(rxData & other) {
			rxData &rt = this->doOp(other, [](T a, T b) { return (a + b); });
			rt.op = "+";
			return (rt);
		}

		void subNone(void *dep) {
			this->subscriber.insert(std::pair<rxData*,std::function<void (T)> >((rxData*)dep, [](T){}));
		}

		void clear() {
			if (this->subscriber.size() == 0 && this->isDyn && !this->unFree)
				delete this;
		}

		void unobserve(int id) {
			this->observer.erase(id);
			this->clear();
		}

		void unsubscribe(rxData *ptr) {
			this->subscriber.erase(ptr);
			this->clear();
		}

		void notify(T value) {
			bool	notify = (this->value_ != value);
			this->value_ = value;
			if (notify)
				this->commit();
		}

		void printDependances(int level)
		{
			for (int i = 0; i < level; i++)
				std::cerr << "\t";
			this->printDebug(std::cerr);
			std::cerr << std::endl;
			for (auto it  = this->subscriber.begin(); it != this->subscriber.end(); it++)
				(*(it->first)).printDependances(level + 1);
		}

		void printDependancesReverse(int level)
		{
			for (int i = 0; i < level; i++)
				std::cerr << "\t";
			this->printDebug(std::cerr);
			std::cerr << std::endl;
			for (auto it  = this->subject.begin(); it != this->subject.end(); it++)
				(*it)->printDependancesReverse(level + 1);
		}

		template <typename U>
		U &operator=(rxData &value) {
			U *rt = new rxData<U>();
			value.subscribe(rt, [rt](T data){
				rt->notify(data.getValue().convert());
			});
			return (*rt);
		}

};

template<typename T>
int rxData<T>::counter = 0;
