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

import android.app.Activity;
import android.bluetooth.*;
import android.graphics.Color;
import android.os.*;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.*;
import android.widget.*;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.SeekBar.*;


public class MainActivity extends Activity {

public final static int DATA_RECEIVED = 1,FILE_RECEIVED= 2;

SeekBar frequenceBar; //Barre frequence 
SeekBar hightResolutionBar; //Barre plus precise
TextView frequenceText;

ListView musicList;
List<Map<String,String>> listItems = new ArrayList<Map<String,String>>();

final String[] fromMapKey = new String[] {"File","Description"};
final int[] toLayoutId = new int[] {android.R.id.text1, android.R.id.text2};

SimpleAdapter adapter; //Adapter pour la liste de fichier

ToggleButton fmControl;
double basefrequency = 87;
double resolution = 0.5;
double frequency = basefrequency+resolution;

BluetoothListener listener;

public Handler mHandler = new Handler() {
	@Override
	public void handleMessage(Message msg) {
		switch(msg.what){
			case DATA_RECEIVED:
				Toast.makeText(getApplicationContext(),(String) msg.obj,200).show();
			break;
			case FILE_RECEIVED:
				String file [] = ((String)msg.obj).split("!");
				Toast.makeText(getApplicationContext(),"Ajout de "+file[0],200).show();
				addFile(file[0],file[1]);
				adapter.notifyDataSetChanged();
			default:
				
			break;
		}
	}
};


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Bundle extras = getIntent().getExtras();
        BluetoothDevice device = (BluetoothDevice)extras.getParcelable("bDevice");
		listener = new BluetoothListener(device,mHandler);
		if(listener.connect()){
			Toast.makeText(getApplicationContext(),"Connexion reussie",500).show();
		}else{
			Toast.makeText(getApplicationContext(),"Connexion echouée",500).show();
			finish();
		}
        setContentView(R.layout.activity_main);
        //addFile("sound.wav","Stars wars");
        this.adapter = new SimpleAdapter(getApplicationContext(),listItems,
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
        frequenceBar   = (SeekBar) findViewById(R.id.FrequenceBar);
        hightResolutionBar = (SeekBar)findViewById(R.id.HightResolution);
        frequenceText  = (TextView)findViewById(R.id.FrequenceText);
        musicList      = (ListView)findViewById(R.id.MusicList);
        fmControl      = (ToggleButton)findViewById(R.id.FMControl);
        musicList.setAdapter(adapter);
        initListener();
        listener.start();
    }
    
    
    //Ajouter un fichier
    public void addFile(String name,String Desc){
		Map<String, String> file = new HashMap<String, String>();
		file.put(fromMapKey[0], name);
		file.put(fromMapKey[1], Desc);
		listItems.add(Collections.unmodifiableMap(file));
	}
    //Initialiser les listeners
    public void initListener(){
    	//Choisir une musiques
    	musicList.setOnItemClickListener(new OnItemClickListener(){
			@Override
			public void onItemClick(AdapterView<?> parent, View view,
					int position, long id) {
				if(listener!=null){
					listener.writeBytes("PLAYID:"+Integer.toString((int)id));
				}
			}
			
		});
    	//Button off/on
    	fmControl.setOnClickListener(new OnClickListener(){
			@Override
			public void onClick(View v) {
				if(listener!=null){
					if(fmControl.isChecked())
						listener.writeBytes(String.format("ON:%1$.1f",frequency));
					else
						listener.writeBytes("OFF");
				}
			}
    	});
    	//Bar de frequence
    	frequenceBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

        	@Override
			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser){
				basefrequency = Math.round((progress*0.205)*100)/100;
				frequency = 87.5+basefrequency+resolution;
				frequenceText.setText(String.format("%1$.1f Mhz",frequency));
			}

			@Override
			public void onStartTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				
			}

			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				
			}
			
			
		});
        //Bar de precision
        hightResolutionBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

        	@Override
			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser){
				resolution = Math.round(progress/10)*0.1;
				frequency = 87.5+basefrequency+resolution;
				frequenceText.setText(String.format("%1$.1f Mhz",frequency));
			}

			@Override
			public void onStartTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				
			}

			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				
			}
		});
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
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
    
    @Override
    public void onBackPressed() {
    	if(listener!=null){
    		listener.close();
    	}
    	finish();
    }   
}
