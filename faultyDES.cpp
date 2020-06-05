#include<bits/stdc++.h>
#include<iostream>
#include<fstream>
using namespace std;

// Main master key - 56 bits + 8 parity bits
int key[64]={
	0,0,0,1,0,0,1,1,
	0,0,1,1,0,1,0,0,
	0,1,0,1,0,1,1,1,
	0,1,1,1,1,0,0,1,
	1,0,0,1,1,0,1,1,
	1,0,1,1,1,1,0,0,
	1,1,0,1,1,1,1,1,
	1,1,1,1,0,0,0,1
};

// Substitution tables...............................
int s[8][4][16] = {
	{
	14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,
	0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,
	4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,
	15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13
  	},

	{
	15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,
	3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,
	0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,
	13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9
	},

	{
	10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,
	13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,
	13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,
	1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12
	},

	{
	7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,
	13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,
	10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,
	3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14
	},

	{
	2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,
	14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,
	4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,
	11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3
	},

	{
	12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,
	10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,
	9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,
	4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13
	},

	{
	4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,
	13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,
	1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,
	6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12
	},

	{
	13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,
	1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,
	7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,
	2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11
	}
};
/*
Here the Correct and incorrect values of last round key bits will 
stored and used for statistical comparison.....................
*/
vector<int *> CorrectL, CorrectR;
vector<int *> FaultyL, FaultyR;

class Des
{
 public:
  int keyi[16][48],total[64],left[32],right[32],ck[28],dk[28],expansion[48],z[48],xor1[48],sub[32],p[32],xor2[32],temp[64],
		pc1[56],ip[64],inv[8][8];

  char final[8*20000];
  void IP();
  void PermChoice1();
  void PermChoice2();
  void Expansion();
  void inverse();
  void xor_two();
  void xor_oneE(int);
  void xor_oneD(int);
  void substitution();
  void permutation();
  void keygen();
  char * Encrypt(char *, int);
  char * Decrypt(char *);
};
// Applying Initial Permutation............................
void Des::IP()
{
	int k=58,i;
	for(i=0;i < 32;i++)
	{
		ip[i]=total[k-1];
		if(k-8>0)  k=k-8;
		else       k=k+58;
	}
	k=57;
	for( i=32;i < 64;i++)
	{
		ip[i]=total[k-1];
		if(k-8 > 0)   k=k-8;
		else	    k=k+58;
	}
}
// PC1 function of round key generation function.....................
void Des::PermChoice1()
{
	int k=57,i;
	for(i=0;i < 28;i++)
	{
		pc1[i]=key[k-1];
		if(k-8 > 0)    k=k-8;
		else	     k=k+57;
	}
	k=63;
	for( i=28;i < 52;i++)
	{
		pc1[i]=key[k-1];
		if(k-8 > 0)    k=k-8;
		else         k=k+55;
	}
	k=28;
	for(i=52;i < 56;i++)
	{
		pc1[i]=key[k-1];
		k=k-8;
	}

}
// Expansion E(A) function of DES f-function.......................
void Des::Expansion()
{
	int exp[8][6],i,j,k;
	for(i=0;i < 8;i++)
	{
		for( j=0;j < 6;j++)
		{
			if((j!=0)||(j!=5))
			{
				k=4*i+j;
				exp[i][j]=right[k-1];
			}
			if(j==0)
			{
				k=4*i;
				exp[i][j]=right[k-1];
			}
			if(j==5)
			{
				k=4*i+j;
				exp[i][j]=right[k-1];
			}
		}
	}
	exp[0][0]=right[31];
	exp[7][5]=right[0];

	k=0;
	for(i=0;i < 8;i++)
	for(j=0;j < 6;j++)
	expansion[k++]=exp[i][j];
}
// Key Generation PC2 function of DES Block Cipher.................
void Des::PermChoice2()
{
	int per[56],i,k;
	for(i=0;i < 28;i++) per[i]=ck[i];
	for(k=0,i=28;i < 56;i++) per[i]=dk[k++];

	z[0]=per[13];z[1]=per[16];z[2]=per[10];z[3]=per[23];z[4]=per[0];z[5]=per[4];z[6]=per[2];z[7]=per[27];
	z[8]=per[14];z[9]=per[5];z[10]=per[20];z[11]=per[9];z[12]=per[22];z[13]=per[18];z[14]=per[11];z[15]=per[3];
	z[16]=per[25];z[17]=per[7];z[18]=per[15];z[19]=per[6];z[20]=per[26];z[21]=per[19];z[22]=per[12];z[23]=per[1];
	z[24]=per[40];z[25]=per[51];z[26]=per[30];z[27]=per[36];z[28]=per[46];z[29]=per[54];z[30]=per[29];z[31]=per[39];
	z[32]=per[50];z[33]=per[46];z[34]=per[32];z[35]=per[47];z[36]=per[43];z[37]=per[48];z[38]=per[38];z[39]=per[55];
	z[40]=per[33];z[41]=per[52];z[42]=per[45];z[43]=per[41];z[44]=per[49];z[45]=per[35];z[46]=per[28];z[47]=per[31];
}
// Xorring with the round key in Encryption side............................
void Des::xor_oneE(int round)
{
	int i;
	for(i=0;i < 48;i++)
		 xor1[i]=expansion[i]^keyi[round-1][i];
}
// Xorring with the round key in Decryption side............................
void Des::xor_oneD(int round)
{
	int i;
	for(i=0;i < 48;i++)
		 xor1[i]=expansion[i]^keyi[16-round][i];
}
// Substitution in DES f-function.............................
void Des::substitution()
{
	int a[8][6],k=0,i,j,p,q,g=0,v;

	for(i=0;i < 8;i++)
	{
		for(j=0;j<6;j++)
		{
			a[i][j]=xor1[k++];
		}
	}

	for( i=0;i < 8;i++)
	{
		p=1;q=0;
		k=(a[i][0]*2)+(a[i][5]*1);
		j=4;
		while(j > 0)
		{
			q=q+(a[i][j]*p);
			p=p*2;
			j--;
		}

		v = s[i][k][q];  // Substitution table lookup value assigned........

		int d,i=3,a[4];
		while(v > 0)
		{
			d=v%2;
			a[i--]=d;
			v=v/2;
		}
		while(i >= 0)
		{
			a[i--]=0;
		}

		for(i=0;i < 4;i++)
		sub[g++]=a[i];
	}
}
// Applying permutation in the DES f- function...............
void Des::permutation()
{
	p[0]=sub[15];p[1]=sub[6];p[2]=sub[19];p[3]=sub[20];p[4]=sub[28];p[5]=sub[11];p[6]=sub[27];p[7]=sub[16];
	p[8]=sub[0];p[9]=sub[14];p[10]=sub[22];p[11]=sub[25];p[12]=sub[4];p[13]=sub[17];p[14]=sub[30];p[15]=sub[9];
	p[16]=sub[1];p[17]=sub[7];p[18]=sub[23];p[19]=sub[13];p[20]=sub[31];p[21]=sub[26];p[22]=sub[2];p[23]=sub[8];
	p[24]=sub[18];p[25]=sub[12];p[26]=sub[29];p[27]=sub[5];p[28]=sub[21];p[29]=sub[10];p[30]=sub[3];p[31]=sub[24];
}
// Xor with left subbits of previous rounds..................
void Des::xor_two()
{
	int i;
	for(i=0;i < 32;i++)
	{
		xor2[i]=left[i]^p[i];
	}
}
// Applying Initial Permutation Inverse at the end....................
void Des::inverse()
{
	int p=40,q=8,k1,k2,i,j;
	for(i=0;i < 8;i++)
	{
		k1=p;k2=q;
		for(j=0;j < 8;j++)
		{
			if(j%2==0)
			{
				inv[i][j]=temp[k1-1];
				k1=k1+8;
			}
			else if(j%2!=0)
			{
				inv[i][j]=temp[k2-1];
				k2=k2+8;
			}
		}
		p=p-1;q=q-1;
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
// Main Encrypt function with additional fault parameter(0/1)...............................
char * Des::Encrypt(char *Text1, int faulty = 0)
{
  int i,a1,j,nB,m,iB,k,K,B[8],n,t,d,round;
  char *Text=new char[20000];
  strcpy(Text,Text1);
  i=strlen(Text);
  int mc=0;
  a1=i%8;
	if(a1!=0) for(j=0;j < 8-a1;j++,i++) Text[i]=' '; Text[i]='\0';
	keygen();
  for(iB=0,nB=0,m=0;m < (strlen(Text)/8);m++)
  {
		for(iB=0,i=0;i < 8;i++,nB++)
		{
			n=(int)Text[nB];
			for(K=7;n >= 1;K--)
			{
				B[K]=n%2;
				n/=2;
			} for(;K >= 0;K--) B[K]=0;
			for(K=0;K < 8;K++,iB++) total[iB]=B[K];
		}
		IP();
		for(i=0;i < 64;i++) total[i]=ip[i];
		for(i=0;i < 32;i++) left[i]=total[i];
		for(;i < 64;i++) right[i-32]=total[i];   
		for(round=1;round <= 16;round++)
		{
			Expansion();
			xor_oneE(round);
			
			// Perturbation only for the last round.................
			if(round==16 && faulty)
				Perturb();

			substitution();

			if(round==16 && faulty)
				AntiPerturb();
			//-----------------------------------------------

			permutation();

			xor_two();
			for(i=0;i < 32;i++) left[i]=right[i];
			for(i=0;i < 32;i++) right[i]=xor2[i];

			// Round 16 outputs stored for comparison-------------------
			if(round==16){
				if(faulty == 0){			
					CorrectL.push_back(left);
					CorrectR.push_back(right);
				}
				else{
					FaultyL.push_back(left);
					FaultyR.push_back(right);
				}
			}
		}

		for(i=0;i < 32;i++) temp[i]=right[i];
		for(;i < 64;i++) temp[i]=left[i-32];
		inverse();

		for(int i=0;i<8;i++){
			for(int j=0;j<8;j++){
				final[mc++] = char(inv[i][j] + '0');
			}
		}
	}
  delete Text;
  return(final);
}

char * Des::Decrypt(char *Text1)
{
  int i,a1,j,nB,m,iB,k,K,B[8],n,t,d,round;
  char *Text = new char[8*20000];
  unsigned char ch;
  strcpy(Text,Text1);
  i=strlen(Text);
	keygen();
	int mc=0;

  for(iB=0,nB=0,m=0;m < (strlen(Text)/64);m++)
  {
	  for(iB=0,i=0;i < 64;i++,nB++)
		{
			 ch = Text[nB];
			 total[iB++] = ch-'0';
		}
		IP();
		for(i=0;i < 64;i++) total[i]=ip[i];
		for(i=0;i < 32;i++) left[i]=total[i];
		for(;i < 64;i++) right[i-32]=total[i];
		for(round=1;round <= 16;round++)
		{
			Expansion();
			xor_oneD(round);
			substitution();
			permutation();
			xor_two();
			for(i=0;i < 32;i++) left[i]=right[i];
			for(i=0;i < 32;i++) right[i]=xor2[i];
		}
		for(i=0;i < 32;i++) temp[i]=right[i];
		for(;i < 64;i++) temp[i]=left[i-32];
		inverse();
		k=128;   d=0;
		for(i=0;i < 8;i++)
		{
		  for(j=0;j < 8;j++)
		  {
			 d=d+inv[i][j]*k;
			 k=k/2;
		  }
			final[mc++]=(char)d;

			k=128;   d=0;
		}
  }   
  final[mc]='\0';
  delete Text;
  return(final);
}

// File Exists or not utility function for I/O puposes........
int cfileexists(const char * filename){
    /* try to open file to read */
    FILE *file;
    if (file = fopen(filename, "r")){
        fclose(file);
        return 1;
    }
    return 0;
}

int main()
{
	freopen("output.txt", "w", stdout); // Check outputs in this file..	

	Des d1, d2;
	char *str = new char[20000];
	char *str1;
	char *str2;
	
	// Forming a random string......................
	// Readable ASCII Characters list[A-Z, a-z, 0-9, Symbols] 
	int lower = 32;
	int upper = 126;
	// Diversity of characters = 126-32+1 = 95
	// More is the diversity, more are the chances of attack (In Report).
	
	srand(time(0));
	int tt = 100;
	int totalTrials = tt;
	int success = 0; // Value out of 100 trails................

	while(tt--){

		for(int i=0;i<19900;i++) // Using ~20000 as string length....
			str[i] = char((rand()%(upper-lower+1)) + lower);
		
		str1 = d1.Encrypt(str, 0); // Without fault

		// cout<<"Input String :\n"<<str<<"\n";
		// cout<<"\nDecrypted plain text :\n"<< d2.Decrypt(str1)<< endl;
		
		Des d3;
		str2 = d3.Encrypt(str, 1); // With fault

		int totalPairs = CorrectL.size(); // Total pairs for comparison
		int xors[32];					// Xors of faulty and Correct R's
		int cnts[32];
		int S_Boxes[8];

		Des d4;
		for(int i=0;i<32;i++)
			d4.sub[i] = i;			// Customising substitution values so that
													// inverse permutation could be found out..				

		d4.permutation();			// For inverse permutation
		d4.keygen();					// Actual keys for last Round generated.....

		vector<int> actual_key[8];

		for(int i=0;i<8;i++){
			for(int j=0;j<6;j++){
				actual_key[i].push_back(d4.keyi[15][6*i + j]); 
			}
		}
		//---------------------------------------------------------
		if(!cfileexists("actualKey.txt")){
			ofstream myfile("actualKey.txt");
			myfile<<"Round 16 Actual Keys:\n\n";

			for(int i=0;i<8;i++){
				myfile<<"Sub-bit "<<i<<": ";
				for(auto it: actual_key[i])
					myfile<<it<<" ";
				myfile<<"\n\n";
			}
			myfile.close();
		}
		//---------------------------------------------------------
		
		vector<vector<int>> predicted_keys[8]; // Storing predicted keys
																					// for each of 8 sub-bits..

		for(int i=0; i < totalPairs; i++){

			for(int j=0;j<32;j++){

				xors[j] = CorrectR[i][j] ^ FaultyR[i][j];

				cnts[d4.p[j]] = xors[j];			// Inverse permutation....

				d4.right[j] = CorrectL[i][j]; // Left Output is unchanged and equal
																			// to Prev Round R.
			}

			d4.Expansion();									// Left Output Expanded......

			for(int j=0;j<8;j++){						// Each of 8 sub-bits

				for(int k=0 ; k<=3 ; k++){

					if(cnts[4*j + k]){ // Check if any of the 4 bits is 1.

						vector<int> result;

						for(int ind = 0; ind<6; ind++){

							result.push_back(d4.expansion[6*j + ind]); // KEY FOUND!!!
						}

						if(predicted_keys[j].size()){   // Check if found previously
																					// for non redundant storage..

							int anyEqual = 0;
							
							for(auto it: predicted_keys[j]){
								int ind = 0;
								for(; ind<6; ind++){
									if(it[ind] != result[ind]){
										break;
									}
								}

								if(ind==6){
									anyEqual = 1;
									break;
								}
							}

							if(!anyEqual){					// If not found, then push_back...
								predicted_keys[j].push_back(result);
							}
						}

						else{
							predicted_keys[j].push_back(result);
						}
						break;
					}
				}
			}
		}

		cout<<"\n\n";
		for(int i=0;i<8;i++)
			cout<<predicted_keys[i].size()<<" ";

		for(int i=0;i<8;i++){
			if(predicted_keys[i].size()){
				success++;

				cout<<"\nActual Key at i = "<<i<<" :\n";

				for(int j=0;j<6;j++){

					cout<<actual_key[i][j]<<" ";
				}

				cout<<"\nPredicted Keys at i = "<<i<<" :\n";

				for(auto it: predicted_keys[i]){

					for(int j=0;j<6;j++){

						cout<<it[j]<<" ";
					}
					cout<<"\n";
				}
			}
		}
	}

	cout<<"\n------------\nSuccess Rate: "<<success<<"/"<<totalTrials<<"\n";

	// cout<<"\n";
	// for(int i=0;i<32;i++)
	// 	cout<<CorrectR[0][i];

	// cout<<"\n";
	// for(int i=0;i<32;i++)
	// 	cout<<FaultyR[0][i];
	delete str;

	return 0;
}

// Keygen algo......................
void Des::keygen()
{
	PermChoice1();

	int i,j,k=0;
	for(i=0;i < 28;i++)
	{
		ck[i]=pc1[i];
	}
	for(i=28;i < 56;i++)
	{
		dk[k]=pc1[i];
		k++;
	}
	int noshift=0,round;
	for(round=1;round<=16;round++)
	{
		if(round==1||round==2||round==9||round==16)
			noshift=1;
		else
			noshift=2;
		while(noshift > 0)
		{
			int t;
			t=ck[0];
			for(i=0;i < 28;i++)
			ck[i]=ck[i+1];
			ck[27]=t;
			t=dk[0];
			for(i=0;i < 28;i++)
			dk[i]=dk[i+1];
			dk[27]=t;
			noshift--;
		}
		PermChoice2();
		for(i=0;i < 48;i++)
			keyi[round-1][i]=z[i];
	}
}
