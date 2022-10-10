#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
using namespace std;

class BTree {
public:
	FILE* bfile;
	const char* fName;
	int header[3];
	int* block;
	int blockL;
	int* road;
	long Tsize = 0;

	BTree(const char* fileName, int blockSize) {
		road = new int[0];
		fName = fileName;
		bfile = fopen(fName, "wb");
		header[0] = blockSize;
		header[1] = 0;
		header[2] = 0;
		fwrite(&header, sizeof(int), 3, bfile);
		fclose(bfile);
		blockL = blockSize / 4;
		block = new int[blockL + 2];
		for (int i = 0; i < blockL + 2; i++) {
			block[i] = 0;
		}
	}

	BTree(const char* fileName) {
		road = new int[0];
		fName = fileName;
		bfile = fopen(fileName, "rb");
		int a;
		fseek(bfile, 0, SEEK_SET);
		for (int i = 0; i < 3; i++) {
			fread(&a, sizeof(int), 1, bfile);
			header[i] = a;
		}
		fclose(bfile);
		blockL = header[0] / 4;
		block = new int[blockL + 2];
	}

	void BNum() {
		bfile = fopen(fName, "rb");
		fseek(bfile, 0, SEEK_END);
		Tsize = (ftell(bfile) - 12) / header[0];
		rewind(bfile);
		fclose(bfile);
	}

	void searchB(int key) {
		road = new int[header[2] + 1];
		road[0] = 1;
		if (header[2] == 0) {
			return;
		}
		int nextB = header[1];
		int depth = 0;
		road[depth] = nextB;
		while (depth < header[2]) {
			bfile = fopen(fName, "rb");
			fseek(bfile, 12 + (nextB - 1) * header[0], SEEK_SET);
			for (int i = 0; i < blockL; i++) {
				int a;
				fread(&a, sizeof(int), 1, bfile);
				block[i] = a;
			}
			fclose(bfile);

			bool flg = false;
			for (int i = 1; i <= blockL - 2; i += 2) {
				if (key < block[i] || block[i] == 0) {
					flg = true;
					nextB = block[i - 1];
					break;
				}
			}
			if (!flg) nextB = block[blockL - 1];
			depth++;
			road[depth] = nextB;
		}
	}

	void insert(int key, int value) {
		block = new int[blockL + 2];
		bfile = fopen(fName, "r+b");
		int a;
		fseek(bfile, 0, SEEK_SET);
		for (int i = 0; i < 3; i++) {
			fread(&a, sizeof(int), 1, bfile);
			header[i] = a;
		}
		fclose(bfile);

		if (header[1] == 0) {
			for (int i = 0; i < blockL + 2; i++) {
				block[i] = 0;
			}
			block[0] = key;
			block[1] = value;
			header[1] = 1;
			bfile = fopen(fName, "r+b");
			fseek(bfile, 0, SEEK_SET);
			fwrite(&header, sizeof(int), 3, bfile);
			for (int i = 0; i < blockL; i++) {
				int a = block[i];
				fwrite(&a, sizeof(int), 1, bfile);
			}
			fclose(bfile);
		}

		else {

			searchB(key);//leaf노드 찾기
			for (int i = 0; i < blockL + 2; i++) {
				block[i] = 0;
			}//block배열 초기화
			bfile = fopen(fName, "r+b");
			fseek(bfile, 12 + (road[header[2]] - 1) * header[0], SEEK_SET);
			for (int i = 0; i < blockL; i++) {
				int a;
				fread(&a, sizeof(int), 1, bfile);
				if (i == blockL - 1) block[i + 2] = a;//마지막 포인터 두칸 뒤로 미루기
				else block[i] = a;
			}
			fclose(bfile);

			int i = 0;
			for (i = 0; i < blockL; i += 2) {
				if (key < block[i] || block[i] == 0) {
					break;
				}
			}
			if (blockL >= i + 2) {
				for (int j = blockL; j >= i + 2; j--) {
					block[j] = block[j - 2];
				}
			}
			block[i] = key;
			block[i + 1] = value;//leaf노드 속 알맞는 위치에 넣기

			//꽉 찬 상태에서 넣었을 경우 (block[blockSize/4]!=0) 나누어 주어야함
			if (block[blockL] != 0) {

				int* Nblock = new int[blockL];
				for (int i = 0; i < blockL; i++) {
					Nblock[i] = 0;
				}
				int j = 0;
				if ((blockL / 2) % 2 == 0) {
					for (int i = blockL / 2 + 2; i < blockL + 1; i++) {
						Nblock[j] = block[i];
						j++;
					}
					Nblock[blockL - 1] = block[blockL + 1];
					for (int i = blockL / 2 + 2; i < blockL + 2; i++) {
						block[i] = 0;
					}
					BNum();
					block[blockL - 1] = Tsize + 1;
				}
				else {
					for (int i = blockL / 2 + 1; i < blockL + 1; i++) {
						Nblock[j] = block[i];
						j++;
					}
					Nblock[blockL - 1] = block[blockL + 1];
					for (int i = blockL / 2 + 1; i < blockL + 2; i++) {
						block[i] = 0;
					}
					BNum();
					block[blockL - 1] = Tsize + 1;
				}

				bfile = fopen(fName, "r+b");
				fseek(bfile, 12 + (road[header[2]] - 1) * header[0], SEEK_SET);
				for (int i = 0; i < blockL; i++) {
					int a = block[i];
					fwrite(&a, sizeof(int), 1, bfile);
				}
				fseek(bfile, 0, SEEK_END);
				for (int i = 0; i < blockL; i++) {
					int a = Nblock[i];
					fwrite(&a, sizeof(int), 1, bfile);
				}
				fclose(bfile);
				////부모노드 만들기
				int dp = header[2];
				int k;
				int LBID, RBID;
				k = Nblock[0];
				RBID = Tsize + 1;
				LBID = road[dp];

				while (dp >= 0) {


					if (dp == 0) {//root를 만들어야 한다면
						int* Rblock = new int[blockL];
						for (int i = 0; i < blockL; i++) {
							Rblock[i] = 0;
						}
						Rblock[0] = LBID;
						Rblock[1] = k;
						Rblock[2] = RBID;
						BNum();
						header[1] = Tsize + 1;
						header[2]++;
						bfile = fopen(fName, "r+b");
						fseek(bfile, 0, SEEK_SET);
						fwrite(&header, sizeof(int), 3, bfile);
						fseek(bfile, 0, SEEK_END);
						for (int i = 0; i < blockL; i++) {
							int a = Rblock[i];
							fwrite(&a, sizeof(int), 1, bfile);
						}
						fclose(bfile);
						break;
					}
					//internal node삽입

					bfile = fopen(fName, "r+b");
					for (int i = 0; i < blockL + 2; i++) {
						block[i] = 0;
					}
					dp--;
					fseek(bfile, 12 + (road[dp] - 1) * header[0], SEEK_SET);
					for (int i = 0; i < blockL; i++) {
						int a;
						fread(&a, sizeof(int), 1, bfile);
						block[i] = a;
					}
					fclose(bfile);

					int i;
					for (i = 1; i < blockL + 2; i += 2) {
						if (k < block[i] || block[i] == 0) {
							break;
						}
					}
					for (int j = blockL + 1; j >= i + 2; j--) {
						block[j] = block[j - 2];
					}
					block[i] = k;
					block[i + 1] = RBID;//internal노드 속 알맞는 위치에 넣기

					if (block[blockL + 1] == 0) {
						bfile = fopen(fName, "r+b");
						fseek(bfile, 12 + (road[dp] - 1) * header[0], SEEK_SET);
						for (int i = 0; i < blockL; i++) {
							int a = block[i];
							fwrite(&a, sizeof(int), 1, bfile);
						}
						fclose(bfile);
						break;
					}
					else {
						for (int i = 0; i < blockL; i++) {
							Nblock[i] = 0;
						}

						int jj = 0;
						if (((blockL + 2) / 2) % 2 == 0) {
							k = block[(blockL + 2) / 2 + 1];
							for (int i = (blockL + 2) / 2 + 2; i < blockL + 2; i++) {
								Nblock[jj] = block[i];
								jj++;
							}
							for (int i = (blockL + 2) / 2 + 1; i < blockL; i++) {
								block[i] = 0;
							}
						}
						else {
							k = block[(blockL + 2) / 2];
							for (int i = (blockL + 2) / 2 + 1; i < blockL + 2; i++) {
								Nblock[jj] = block[i];
								jj++;
							}
							for (int i = (blockL + 2) / 2; i < blockL; i++) {
								block[i] = 0;
							}
						}

						BNum();
						RBID = Tsize + 1;
						LBID = road[dp];
						bfile = fopen(fName, "r+b");
						fseek(bfile, 12 + (road[dp] - 1) * header[0], SEEK_SET);
						for (int i = 0; i < blockL; i++) {
							int a = block[i];
							fwrite(&a, sizeof(int), 1, bfile);
						}
						fseek(bfile, 0, SEEK_END);
						for (int i = 0; i < blockL; i++) {
							int a = Nblock[i];
							fwrite(&a, sizeof(int), 1, bfile);
						}
						fclose(bfile);
					}

				}
			}
			else {
				block[blockL - 1] = block[blockL + 1];
				bfile = fopen(fName, "r+b");
				fseek(bfile, 12 + (road[header[2]] - 1) * header[0], SEEK_SET);
				for (int i = 0; i < blockL; i++) {
					int a = block[i];
					fwrite(&a, sizeof(int), 1, bfile);
				}
				fclose(bfile);
			}
		}
	}

	void searchValue(const char* IFileName, const char* OFileName) {
		int cnt = 0;
		string key;
		bool flg = false;
		while (1) {
			key = "";
			FILE* input = fopen(IFileName, "rt");
			fseek(input, cnt, SEEK_SET);
			while (1) {
				char c = fgetc(input); cnt++;
				if (c == EOF) {
					flg = true;
					break;
				}
				if (c == '\n')
					break;
				key = key + c;
			}
			cnt++;
			fclose(input);

			searchB(stoi(key));//leaf노드 찾기

			bfile = fopen(fName, "rb");
			fseek(bfile, 12 + (road[header[2]] - 1) * header[0], SEEK_SET);
			for (int i = 0; i < blockL; i++) {
				int a;
				fread(&a, sizeof(int), 1, bfile);
				block[i] = a;
			}
			fclose(bfile);

			int re = 0;
			for (int i = 0; i < blockL - 1; i += 2) {
				if (block[i] == stoi(key)) {
					re = block[i + 1];
					break;
				}
			}
			string out = "";
			out = key + ',' + to_string(re) + "\n";
			FILE* output = fopen(OFileName, "a+t");
			for (int i = 0; i < out.size(); i++) {
				fputc(out[i], output);
			}
			fclose(output);

			if (flg) break;
		}
	}

	void searchRange(int str, int end, const char* OFileName) {
		searchB(str);//leaf노드 찾기
		int BID = road[header[2]];

		string out = "";
		bfile = fopen(fName, "rb");
		while (1) {
			if (BID == 0) break;

			fseek(bfile, 12 + (BID - 1) * header[0], SEEK_SET);
			for (int i = 0; i < blockL; i++) {
				int a;
				fread(&a, sizeof(int), 1, bfile);
				block[i] = a;
			}

			int i;
			for (i = 0; i <= blockL - 2; i += 2) {
				if (block[i] == 0) break;
				if (block[i] >= str && block[i] <= end) {
					out = out + to_string(block[i]) + "," + to_string(block[i + 1]) + " / ";
				}
				if (i == blockL - 3) break;
			}
			if (block[i] == 0) i = i - 2;
			if (block[i] <= end) {
				BID = block[blockL - 1];
			}
			else break;
		}
		out = out + "\n";
		fclose(bfile);

		FILE* output = fopen(OFileName, "a+t");
		for (int i = 0; i < out.size(); i++) {
			fputc(out[i], output);
		}
		fclose(output);

	}

	void printAll(const char* OFileName) {
		vector<vector<int>> bid;
		vector<int> v;
		bid.push_back(v);
		bid[0].push_back(header[1]);
		int dp = 0;
		for (int i = 0; i < blockL; i++) {
			block[i] = 0;
		}



		bfile = fopen(fName, "rb");
		while (dp < header[2]) {//internal bid넣기

			for (int i = 0; i < bid[dp].size(); i++) {
				fseek(bfile, 12 + (bid[dp][i] - 1) * header[0], SEEK_SET);
				for (int i = 0; i < blockL; i++) {
					int a;
					fread(&a, sizeof(int), 1, bfile);
					block[i] = a;
				}
				bid.push_back(v);
				for (int i = 0; i < blockL; i += 2) {
					if (block[i] != 0) bid[dp + 1].push_back(block[i]);
				}
			}
			dp++;
		}
		fclose(bfile);
		dp = 0;
		while (dp < header[2]) {
			string out = "[level " + to_string(dp) + "]\n";

			for (int i = 0; i < bid[dp].size(); i++) {
				bfile = fopen(fName, "rb");
				fseek(bfile, 12 + (bid[dp][i] - 1) * header[0], SEEK_SET);
				for (int i = 0; i < blockL; i++) {
					int a;
					fread(&a, sizeof(int), 1, bfile);
					block[i] = a;
				}
				fclose(bfile);//블럭 가져오기

				for (int i = 1; i <= blockL - 2; i += 2) {
					if (block[i] != 0) {
						out = out + to_string(block[i]) + ", ";
					}
				}
			}
			out = out + "\n\n";
			FILE* output = fopen(OFileName, "a+t");
			for (int i = 0; i < out.size(); i++) {
				fputc(out[i], output);
			}
			fclose(output);
			dp++;
		}
		//leaf노드 넣기
		string out = "[level " + to_string(dp) + "]\n";
		for (int i = 0; i < bid[dp].size(); i++) {
			bfile = fopen(fName, "rb");
			fseek(bfile, 12 + (bid[dp][i] - 1) * header[0], SEEK_SET);
			for (int i = 0; i < blockL; i++) {
				int a;
				fread(&a, sizeof(int), 1, bfile);
				block[i] = a;
			}
			fclose(bfile);//블럭 가져오기

			for (int i = 0; i <= blockL - 3; i += 2) {
				if (block[i] != 0) {
					out = out + to_string(block[i]) + ", ";
				}
			}
		}
		out = out + "\n\n";
		FILE* output = fopen(OFileName, "a+t");
		for (int i = 0; i < out.size(); i++) {
			fputc(out[i], output);
		}
		fclose(output);
	}
};
int main(int argc, char* argv[]) {


	char command = argv[1][0];
	BTree* myBtree;

	switch (command)
	{
	case 'c':
		myBtree = new BTree(argv[2], stoi(argv[3]));
		break;
	case 'i':
	{
		myBtree = new BTree(argv[2]);
		int cnt = 0;
		string key, value;
		bool flg = false;
		while (1) {
			FILE* input = fopen(argv[3], "rt");
			fseek(input, cnt, SEEK_SET);
			while (1) {
				char c = fgetc(input); cnt++;
				if (c == EOF) {
					flg = true;
					break;
				}
				if (c == ',')
					break;
				key = key + c;
			}
			fseek(input, 0, SEEK_CUR);
			while (1) {
				char c = fgetc(input); cnt++;
				if (c == EOF) {
					flg = true;
					break;
				}
				if (c == '\n')
					break;
				value = value + c;
			}
			fclose(input);
			myBtree->insert(stoi(key), stoi(value));
			if (flg) break;
			key = "";
			value = "";
		}
		//insert
		break;
	}
	case 's':
		myBtree = new BTree(argv[2]);
		myBtree->searchValue(argv[3], argv[4]);
		//search
		break;
	case 'r':
	{
		myBtree = new BTree(argv[2]);
		int cnt = 0;
		string key, value;
		bool flg = false;
		while (1) {
			FILE* input = fopen(argv[3], "rt");
			fseek(input, cnt, SEEK_SET);
			while (1) {
				char c = fgetc(input); cnt++;
				if (c == EOF) {
					flg = true;
					break;
				}
				if (c == ',')
					break;
				key = key + c;
			}
			fseek(input, 0, SEEK_CUR);
			while (1) {
				char c = fgetc(input); cnt++;
				if (c == EOF) {
					flg = true;
					break;
				}
				if (c == '\n')
					break;
				value = value + c;
			}
			fclose(input);
			myBtree->searchRange(stoi(key), stoi(value), argv[4]);
			if (flg) break;
			key = "";
			value = "";
		}
		//range search
		break;
	}
	case 'p':
		myBtree = new BTree(argv[2]);
		myBtree->printAll(argv[3]);
		//print all
		break;

	}

}