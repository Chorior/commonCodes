import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.net.Socket;

/**
 * Created by pengzhen on 2017/6/20.
 */

public class SocketClient {
    public static final String IP_ADDR = "127.0.0.1";
    public static final int PORT = 1205;

    public void sendData(String str_data)
    {
        try {
            InetSocketAddress addr = new InetSocketAddress(IP_ADDR, PORT);
            Socket sock = new Socket();
            sock.connect(addr);
			
            String str_verify = "verify";			
            byte[] byte_verify = StringToByteArray(str_verify);
			byte[] byte_length = IntToByteArray(str_data.length());
			byte[] verify_data = new byte[10];
			System.arraycopy(byte_verify,0, verify_data, 0, byte_verify.length);
			System.arraycopy(byte_length,0, verify_data, byte_verify.length, byte_length.length);
            sock.getOutputStream().write(verify_data);

            byte[] byte_reply = new byte[3];
            int length = sock.getInputStream().read(byte_reply);
            String str_reply = ByteArrayToString(byte_reply,length);
            if("OK".equals(str_reply))
            {				
                byte[] byte_data = StringToByteArray(str_data);
                sock.getOutputStream().write(byte_data);
            }
        }catch (IOException e) {
            //Log.i("TAG", "client connect failed.");
			System.out.println("client connect failed.");
            e.printStackTrace();
        }
    }

    private String ByteArrayToString(byte[] arr, int length)
    {
        String result=null;  
        int index = 0;  
        while(index < length) {  
            if(arr[index] == 0) {  
                break;  
            }  
            index++;  
        }  
        byte[] temp = new byte[index];  
        System.arraycopy(arr, 0, temp, 0, index);  
		
        try {  
            result= new String(temp,"GBK");  
        } catch (UnsupportedEncodingException e) {  
            e.printStackTrace();  
        }  
        return result; 
    }

    private byte[] StringToByteArray(String str)
	{
        byte[] temp = null;
        try {
            temp = str.getBytes("GBK");
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
        return temp;
    }
	
	private byte[] IntToByteArray(int n) {  
        byte[] b = new byte[4];  
        b[0] = (byte) (n & 0xff);  
        b[1] = (byte) (n >> 8 & 0xff);  
        b[2] = (byte) (n >> 16 & 0xff);  
        b[3] = (byte) (n >> 24 & 0xff);  
        return b;  
    } 
	
	private int ByteArrayToInt(byte[] bArr) 
	{  
		if(bArr.length!=4){
			return -1;
		}  
		 
		return (int) ((((bArr[3] & 0xff) << 24)    
				| ((bArr[2] & 0xff) << 16)    
				| ((bArr[1] & 0xff) << 8) 
				| ((bArr[0] & 0xff) << 0)));   
    } 
	
	public static void main(String[] args)
	{
		SocketClient client = new SocketClient();		
		client.sendData("ada");
	}
}
