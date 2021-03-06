/*
* Copyright (C) 2014 Thomas DUBIER
*
* This file is part of RPiHackFM.
*
* RPiHackFM is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* RPiHackFM is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with RPiHackFM. If not, see <http://www.gnu.org/licenses/>.
*/
package com.example.rpihackfm;

import java.io.*;
import java.util.UUID;

import android.bluetooth.*;
import android.os.*;

public class BluetoothListener extends Thread{
	private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
	private BluetoothSocket socket = null;
	private InputStream inStream = null;
	private OutputStream outStream = null;
	private Handler mHandle;
	BluetoothDevice device;
	boolean close = true;
	
	public BluetoothListener(BluetoothDevice device,Handler mHandle){
		this.device = device;
		this.mHandle = mHandle;
	}
	@Override
	public void run(){
		byte buffer [] = new byte[256];
		int bytes;
		
		writeBytes("H4ckP1FM"); //Identification
		
		while(!close){
			try{
				int length = inStream.read();
				String message = "";
				while(length>0){
					bytes = inStream.read(buffer);
					String data = new String(buffer,0,bytes);
					message+=data;
					length-=bytes;
				}
				if(message.contains("FILE:"))
					mHandle.obtainMessage(MainActivity.FILE_RECEIVED,message.split(":")[1]).sendToTarget();
				else
					mHandle.obtainMessage(MainActivity.DATA_RECEIVED,message).sendToTarget();
			}catch(Exception e){
				try{
					socket.close();
				}catch(Exception ec){}
				close = true;
			}
		}
	}
	
	public boolean writeBytes(String msg) {
        try {
        	byte [] bytes = msg.getBytes();
        	/*outStream.write(bytes.length);
        	try{
        		Thread.sleep(10);
        	}catch(Exception e){}*/
            outStream.write(bytes);
        } catch (IOException e) {
        	return false;
        }
        return true;
    }

	public boolean connect(){
		try{
			socket = device.createRfcommSocketToServiceRecord(MY_UUID);
			socket.connect();
			inStream = socket.getInputStream();
			outStream= socket.getOutputStream();
			close = false;
		}catch(Exception e){
			try{
				socket.close();
			}catch(Exception ex){}
			return false;
		}
		return true;
	}
	
	public boolean close(){
		try{
			close = true;
			socket.close();
		}catch(Exception e){
			return false;
		}
		return true;
	}
	
	public BluetoothSocket getSocket(){
		return socket;
	}
}
