
#include "lookup.h"
#include<bits/stdc++.h>
#include "des.h"
using namespace std;
using namespace round_func;

// strlen function to find length of a string ending with \r\n
int strlen(char *str){
	int len=0;
	while(*str!='\r'||*(str+1)!='\n')len++,str++;
	return len;
}



/*
Here the Correct and incorrect values of last round key bits will 
stored and used for statistical comparison.....................
*/
vector<int *> CorrectL, CorrectR;
vector<int *> FaultyL, FaultyR;

// Initial Permutation Function
void Des::IP()
{
	for(int i=0;i<64;i++){
		ip[i]= total[ip_table[i]-1];
	}
}

// Applying Initial Permutation inverseIP at the end....................
void Des::inverseIP()
{
	for(int i=0;i<64;i++){
		inv_ip[ip_table[i]-1] = temp[i]; 
	}
}

// PC1 function of round key generation function.....................
void Des::PermChoice1(){
	for(int i=0;i<56;i++){
		pc1[i] = key[keygen::pc1_table[i]-1];
	}
}

// Expansion E(A) function of DES f-function.......................
// Expands 32 bit right subpart to 48 bit
void Des::Expansion(){
	for(int i=0;i<48;i++){
		expansion[i] = right[expansion_table[i]-1];
	}	
}

// Key Generation PC2 function of DES Block Cipher.................
void Des::PermChoice2()
{
	int per[56];
	for(int i=0;i<28;i++)per[i]=ck[i],per[i+28]=dk[i];

	for(int i=0;i<48;i++)
		round_key[i] = per[keygen::pc2_table[i]-1];
}

/* 
Xorring with the round key
For decryption the round keys should be used in reverse order
as generated by keygen */
void Des::xor_key(int round, int type)
{
	round--;
	if(type == DEC) round = 15 - round;
	for(int i=0;i < 48;i++)
		 xor1[i]=expansion[i]^keyi[round][i];
}

// Substitution in DES f-function. 6*4 function
void Des::substitution(){
	int idx=0;
	for(int i=0;i<48;i+=8){
		int s_box_num = i/8,row=xor1[i]<<1+xor1[i+5],col=0; 
		for(int j=1;j<5;j++){
			col += col<<1+xor1[i+j];
		}
		int num = round_func::s[s_box_num][row][col];
		for(int i=0;i<4;i++)
			sub[s_box_num*4+3-i]=num&1,num>>=1;
	}
}

// Applying permutation in the DES f- function...............
void Des::permutation(){
	for(int i=0;i<32;i++){
		p[i]=sub[round_func::permutation_table[i]-1];
	}
}

// Xor with left subbits of previous rounds..................
void Des::xor_left()
{
	for(int i=0;i < 32;i++){
		xor2[i]=left[i]^p[i];
	}
}

// Here we perturb one bit(at [0][0]) of each S-Box of last rounds.
void Perturb(){
	for(int i=0;i<8;i++){

		s[i][0][0] = (s[i][0][0] + 1) % 16;
	}
}
// Here we reverse back those changes.....................
void AntiPerturb(){
	for(int i=0;i<8;i++){

		s[i][0][0] = (s[i][0][0] - 1 + 16) % 16;
	}	
}

void Des::round_function(int round,int type, bool faulty=false){
	Expansion();
	xor_key(round, type);
	if(type == ENC){
		// Perturb s block only during encryption
		// Perturbation only for the last round.................
		if (round == 16 && faulty)
			Perturb();

		substitution();

		if (round == 16 && faulty)
			AntiPerturb();
		
	}
	else {
		substitution();
	}	
	permutation();
	xor_left();
}
/*
Main run des function identical for both encryption and decryption.
with additional parameter faulty to run encryption with perturbed s box entry 
Input Text is the plaintext or ciphertext. 
Output is a unsigned unsigned character stream after running des algorithm
*/
char * Des::run_des(char *Text,int type, bool faulty = 0)
{
	int len = strlen(Text);
	int size = ceil(len*8 / 64.0)*64; // pad stream to nearest multiple of 64 bits.
	int *Text1 = new int[size],idx=0;
	//convert character stream to bit stream for easy processing
	for(int i=1;i<=len;i++){
		//int ch=(i<=len)?(int)Text[i-1]:' ';
		int ch = Text[i-1];
		//cout<<ch;//Text[i-1];
		for(int j=0;j<8;j++){
			Text1[8*i-j-1]=ch&1;
			ch>>=1;
		}
	}	
	keygen();
	for(int nB=0;nB < size / 64; nB++){

		memcpy(total,Text1+nB*64,sizeof(total));
		IP();
		memcpy(total,ip,sizeof(total));
		memcpy(left,total,sizeof(left));
		memcpy(right,total+32,sizeof(right));  
		for(int round=1;round <= 16;round++)
		{
			round_function(round,type,faulty);
			memcpy(left,right,sizeof(left));
			memcpy(right,xor2,sizeof(right));
			
			// Round 16 outputs stored for comparison-------------------
			if(round==16 && type == ENC){
				if(!faulty){			
					CorrectL.push_back(left);
					CorrectR.push_back(right);
				}
				else{
					FaultyL.push_back(left);
					FaultyR.push_back(right);
				}
			}
		}

		memcpy(temp,right,sizeof(temp));
		memcpy(temp+32,left,sizeof(left));
		
		inverseIP();
		int k = 128,d = 0;
		for (int i = 0; i < 8; i++){
			for (int j = 0; j < 8; j++){
				 d = d + inv_ip[i*8+j] * k;
				 k = k / 2;
			}
			final[idx++] = (char)d;
			k = 128;d = 0;
		}
	}
	final[idx++] = '\r',final[idx++]='\n';
	delete Text1;
	return(final);
}

char * Des::Encrypt(char *Text,int faulty){
	return run_des(Text, ENC, faulty);
}

char * Des::Decrypt(char *Text){
	return run_des(Text, DEC);
}

// Keygen algo......................
void Des::keygen()
{
	PermChoice1();

	int i, j, k = 0;
	for (i = 0; i < 28; i++)
	{
		ck[i] = pc1[i];
	}
	for (i = 28; i < 56; i++)
	{
		dk[k] = pc1[i];
		k++;
	}
	int noshift = 0, round;
	for (round = 1; round <= 16; round++)
	{
		if (round == 1 || round == 2 || round == 9 || round == 16)
			noshift = 1;
		else
			noshift = 2;
		while (noshift > 0)
		{
			int t;
			t = ck[0];
			for (i = 0; i < 28; i++)
				ck[i] = ck[i + 1];
			ck[27] = t;
			t = dk[0];
			for (i = 0; i < 28; i++)
				dk[i] = dk[i + 1];
			dk[27] = t;
			noshift--;
		}
		PermChoice2();
		for (i = 0; i < 48; i++)
			keyi[round - 1][i] = round_key[i];
	}
}