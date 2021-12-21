(*
 *  CS164 Fall 94
 *
 *  Programming Assignment 1
 *    Implementation of a simple stack machine.
 *
 *  Skeleton file
 *)

class Stack {
	items: String;
	contents: Stack;
	getItem(): String {items};

	getContents(): Stack {contents};

	pop(): Stack {contents};

	init (item: String, content: Stack): Stack {
		{		
			items <- item;
			contents <- content;
			self;
		}
	};
	
	insert (item: String, index: Int): Stack {
		let nil: Stack in {
			if (index = 0)
			then { init(item, nil); self; }
			else (new Stack).init(item, self)
			fi;
		}
	};

	stringBuilder (): String {
		{
			if (isVoid contents)
			then	items.concat("\n")
			else	items.concat("\n").concat(contents.stringBuilder())
			fi;
		}
	};
	
	display (): Int {
		{ (new IO).out_string(stringBuilder()); 0; }
	};

	add (): Stack {
		let method: A2I <- (new A2I), val1: Int, val2: Int in {
			val1 <- method.a2i(items);
			val2 <- method.a2i(contents.getItem()) + val1;
			(new Stack).init(method.i2a(val2), contents.getContents());		
		}
	};

	
	swap (): Stack {
		let val2: String, val1: String in {
			val1 <- items;
			val2 <- contents.getItem();
			(new Stack).init(val2, (new Stack.init(val1, contents.getContents())));		
		}
	};
};

class StackCommand inherits IO {
	index: Int <- 0;
  	method: A2I <- (new A2I);
	main: Stack <- (new Stack); 	
	
	execute (a: String): Int {
		{		
			if (a = "e")
			then {
				if (main.getItem() = "+")
				then { 
					main <- main.pop();
					main <- main.add();
				}
				else {
					if (main.getItem() = "s")
					then {
						main <- main.pop();
						main <- main.swap();
					} else ""
					fi;
				}
				fi;				
			}
			else { 
				if (a = "d")
				then main.display()
				else { main <- main.insert(a, index); index <- index + 1; }
				fi;
			}
			fi;
			0;
		}
	};
};


class Main inherits IO {
	stck: Stack;
	nil: Stack;

	main() : Int {
		let command: StackCommand <- (new StackCommand), char: String <- "" in {
			while (not (char = "x")) loop
				{
					out_string(">");
					char <- in_string();
					command.execute(char);
				}
			pool;
			0;
		}
	};
};
