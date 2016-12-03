import com.sun.jna.Library;
import com.sun.jna.Native;

public class SSS7 {
	private interface NativeSSS7 extends Library {
		public int libsss7_start(String serialport);
		public int libsss7_can_send();
		public void libsss7_send(String data);
		public int libsss7_send_failed();
		public int libsss7_has_received();
		public void libsss7_get_received(String data);
		public void libsss7_stop();
	};

	private NativeSSS7 lib;
	private String serial;

	SSS7(String serial) {
		this.lib = (NativeSSS7) Native.loadLibrary("libsss7.so", NativeSSS7.class);
		this.serial = serial;
	}

	public boolean start() {
		return this.lib.libsss7_start(this.serial) == 0;
	}

	public boolean canSend() {
		return this.lib.libsss7_can_send() == 1;
	}

	public boolean sendFailed() {
		return this.lib.libsss7_send_failed() == 1;
	}

	public boolean hasReceived() {
		return this.lib.libsss7_has_received() == 1;
	}

	public String getReceived() {
		String tmp = new String();
		this.lib.libsss7_get_received(tmp);
		return tmp;
	}
}
