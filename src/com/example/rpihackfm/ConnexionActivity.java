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


import android.app.Activity;
import android.bluetooth.*;
import android.content.*;
import android.os.*;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.*;
import android.widget.AdapterView.*;

public class ConnexionActivity extends Activity {

ListView bluetoothDevicesList;
BluetoothBroadcastReceiver bReceiver;
BluetoothAdapter bluetooth = BluetoothAdapter.getDefaultAdapter();

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_connection);
		bluetoothDevicesList = (ListView)findViewById(R.id.listBluetoothDevices);
		bReceiver = new BluetoothBroadcastReceiver(this);
		//Activer le bluetooth
		if(!bluetooth.isEnabled()){
			Intent launchBluetooth = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
			startActivity(launchBluetooth);
		}
		if(bluetooth.isDiscovering()){
			bluetooth.cancelDiscovery();
		}
		bluetooth.startDiscovery();
		
		registerReceiver(bReceiver, new IntentFilter(BluetoothDevice.ACTION_FOUND));
		bluetoothDevicesList.setAdapter(bReceiver.getAdapter());
		initListener();
	}

	public void initListener(){
		bluetoothDevicesList.setOnItemClickListener(new OnItemClickListener(){

			@Override
			public void onItemClick(AdapterView<?> parent, View view,
					int position, long id) {
				BluetoothDevice selectedDevice = bReceiver.getDeviceByIndex((int)id);
				connect(selectedDevice);
			}
			
		});
	}
	
	public void connect(BluetoothDevice device){
		//Toast.makeText(getApplicationContext(),"Connexion à "+device.getName(),500).show();
		Intent fm_activity = new Intent(getApplicationContext(),MainActivity.class);
		fm_activity.putExtra("bDevice",device);
		startActivity(fm_activity);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.connexion, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
}
