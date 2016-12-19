import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.Memory;

public class SSS7 {
	private static SSS7 instance = null;

	public static SSS7 getInstance() {
		if(instance == null) {
			instance = new SSS7();
		}
		return instance;
	}

	private interface NativeSSS7 extends Library {
		public int libsss7_start(String serialport);
		public int libsss7_can_send();
		public void libsss7_send(Pointer data);
		public int libsss7_send_failed();
		public int libsss7_has_received();
		public void libsss7_get_received(Pointer data);
		public void libsss7_stop();
	};

	public final int payloadLength = 16;

	private NativeSSS7 lib;
	private String serial;

	protected SSS7() {
		this.lib = (NativeSSS7) Native.loadLibrary("libsss7.so", NativeSSS7.class);
	}

	public boolean start(String serial) {
		return this.lib.libsss7_start(this.serial) == 0;
	}

	public boolean canSend() {
		return this.lib.libsss7_can_send() == 1;
	}

	public void send(byte[] data) {
		Pointer p = new Memory(this.payloadLength);
		for(long i = 0; i < this.payloadLength; i++) {
			if(i < data.length) {
				p.setByte(i, data[(int) i]);
			}
			else {
				p.setByte(i, (byte) 0);
			}
		}

		this.lib.libsss7_send(p);
	}

	public boolean sendFailed() {
		return this.lib.libsss7_send_failed() == 1;
	}

	public boolean hasReceived() {
		return this.lib.libsss7_has_received() == 1;
	}

	public byte[] getReceived() {
		Pointer p = new Memory(this.payloadLength);
		this.lib.libsss7_get_received(p);
		return p.getByteArray(0, this.payloadLength);
	}

	public void stop() {
		this.lib.libsss7_stop();
	}
}
