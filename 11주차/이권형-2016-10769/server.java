import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Random;
import java.util.concurrent.ThreadLocalRandom;

public class server {

	public static void main(String[] args) throws ClassNotFoundException {
		String rand_num = generate();
		int strike_count = 0;
		int ball_count = 0;
		System.out.println("Random number is: " + rand_num);
		
		try {
			ServerSocket server = new ServerSocket(11001);
			
			while (true) {
				strike_count = 0;
				ball_count = 0;
				Socket aSocket = server.accept();
				
				ObjectInputStream instream = new ObjectInputStream(aSocket.getInputStream());
				ObjectOutputStream outstream = new ObjectOutputStream(aSocket.getOutputStream());

				Object obj = instream.readObject();
				String user_input = obj.toString();
				System.out.println("User entered: " + obj);
				// check strike
				for (int i = 0; i < 3; i++) {
					for (int j = 0; j < 3; j++) {
						if (user_input.charAt(i) == rand_num.charAt(j)) {
							if (i == j)
								strike_count++;
							else 
								ball_count++;
						}
					}
				}
				
				// If 3 strike. You win. restart game.
				if (strike_count == 3) {
					rand_num = generate();
					outstream.writeObject("You win!");
					
					System.out.println("New game started");
					System.out.println("Random number is: " + rand_num);
					continue;
				}
				
				if (obj.toString().equals("000")) {
					rand_num = generate();
					
					outstream.writeObject("New game started");
					System.out.println("New game started");
					System.out.println("Random number is: " + rand_num);
					continue;
				}
				
				outstream.writeObject(strike_count + "S" + ball_count + "B");
				outstream.flush();
				System.out.println(strike_count + "S" + ball_count + "B");
				
				aSocket.close();
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public static String generate() {
		int numbers[] = new int[3];
		Random random = new Random();
		while(numbers[0] == 0){
		      numbers[0] = random.nextInt(10);
	    }	     
		while(numbers[0] == numbers[1] || numbers[1] == 0){
	      numbers[1] = random.nextInt(10);
		}
	    while(numbers[0] == numbers[2] || numbers[1] == numbers[2] || numbers[2] == 0){
	      numbers[2] = random.nextInt(10);
	    }   
		
		return "" + numbers[0] + numbers[1] + numbers[2];
	}

}
