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

import java.util.*;

import android.app.*;
import android.bluetooth.*;
import android.content.*;
import android.graphics.*;
import android.view.*;
import android.widget.*;

public class BluetoothBroadcastReceiver extends BroadcastReceiver{

	Context activityContext;
	Activity activity;
	
	private ArrayList<BluetoothDevice> bluetoothDevices = new ArrayList<BluetoothDevice>();
	List<Map<String,String>> listItems = new ArrayList<Map<String,String>>();
	SimpleAdapter adapter;

	final String[] fromMapKey = new String[] {"Name","State"};
	final int[] toLayoutId = new int[] {android.R.id.text1, android.R.id.text2};
	
	public BluetoothBroadcastReceiver(Activity activity){
		this.activityContext = activity.getApplicationContext();
		this.activity = activity;
		//Adapter pour la list view
		//listItems.add(createDevice("Bidon","Bidon"));
		this.adapter = new SimpleAdapter(activityContext,listItems,
				android.R.layout.simple_list_item_2,
				fromMapKey,
				toLayoutId){
			//Recriture de la fonction getView pour changer les couleurs
			@Override
			public View getView(int position, View convertView, ViewGroup parent) {
			    View view = super.getView(position, convertView, parent);
			    TextView text1 = (TextView) view.findViewById(android.R.id.text1);
			    TextView text2 = (TextView) view.findViewById(android.R.id.text2);
			    text1.setTextColor(Color.BLACK);
			    text2.setTextColor(Color.BLACK);
			    return view;
			  }
		};
	}
	
	
	@Override
	public void onReceive(Context context, Intent intent) {
		String action = intent.getAction();
		if(action.equals(BluetoothDevice.ACTION_FOUND)){
			BluetoothDevice bD = (BluetoothDevice)intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
			//Test si le peripherique bluetooth existe deja en liste
			boolean exist = false;
			for(BluetoothDevice bluetoothDevice : bluetoothDevices){
				if(bluetoothDevice.getAddress().equals(bD.getAddress()))
					exist = true;
			}
			//Si il n'existe pas on l'ajoute
			if(!exist){
				bluetoothDevices.add(bD);
				listItems.add(createDevice(bD.getName(),bD.getAddress()));
				activity.runOnUiThread(new Runnable(){
					@Override
					public void run(){
						adapter.notifyDataSetChanged();
					}
				});
			}
		}
	}
	
	public ArrayList<BluetoothDevice> getBluetoothDevices(){
		return bluetoothDevices;
	}
	
	public BluetoothDevice getDeviceByIndex(int index){
		try{
			return bluetoothDevices.get(index);
		}catch(Exception e){
			return null;
		}
	}
	
	public SimpleAdapter getAdapter(){
		return adapter;
	}
	
	public Map<String,String> createDevice(String name,String key){
		Map<String, String> device = new HashMap<String, String>();
		device.put(fromMapKey[0], name);
		device.put(fromMapKey[1], key);
		return Collections.unmodifiableMap(device);
	}
}
