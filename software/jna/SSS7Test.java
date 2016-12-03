

/** Simple example of native C POSIX library declaration and usage. */
public class SSS7Test {


	public static void main(String[] args) {
		SSS7 bus = new SSS7("/dev/ttyUSB0");

		bus.start();

	}
}
