

/** Simple example of native C POSIX library declaration and usage. */
public class SSS7Test {


	public static void main(String[] args) {
		SSS7 bus = new SSS7("/dev/ttyUSB0");

		bus.start();

		while(!bus.canSend());
		bus.send("Hallo Java".getBytes());
		while(!bus.canSend());


		while(!bus.hasReceived());
		byte[] data  = bus.getReceived();
		String str = new String(data);
		System.out.println(str);


		bus.stop();
	}
}
