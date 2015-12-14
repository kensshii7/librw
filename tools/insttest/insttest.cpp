#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <new>

#include <rw.h>
#include <src/gtaplg.h>

using namespace std;
using namespace rw;

int
main(int argc, char *argv[])
{
	gta::attachPlugins();

	rw::version = 0x34003;
//	rw::version = 0x33002;
	rw::platform = rw::PLATFORM_PS2;
//	rw::platform = rw::PLATFORM_OGL;
//	rw::platform = rw::PLATFORM_XBOX;
//	rw::platform = rw::PLATFORM_D3D8;
//	rw::platform = rw::PLATFORM_D3D9;

	int uninstance = 0;
	int arg = 1;

	if(argc < 2){
		printf("usage: %s [-u] in.dff\n", argv[0]);
		return 0;
	}

	if(strcmp(argv[arg], "-u") == 0){
		uninstance++;
		arg++;
		if(argc < 3){
			printf("usage: %s [-u] in.dff\n", argv[0]);
			return 0;
		}
	}

	Clump *c;
	uint32 len;
	uint8 *data = getFileContents(argv[arg], &len);
	assert(data != NULL);
	StreamMemory in;
	in.open(data, len);

	ChunkHeaderInfo header;
	readChunkHeaderInfo(&in, &header);
	if(header.type == ID_UVANIMDICT){
		UVAnimDictionary *dict = UVAnimDictionary::streamRead(&in);
		currentUVAnimDictionary = dict;
		readChunkHeaderInfo(&in, &header);
	}
	assert(header.type == ID_CLUMP);
	debugFile = argv[arg];
	c = Clump::streamRead(&in);
	assert(c != NULL);

//	printf("%s\n", argv[arg]);

/*
	for(int32 i = 0; i < c->numAtomics; i++){
		Atomic *a = c->atomicList[i];
		Pipeline *ap = a->pipeline;
		Geometry *g = a->geometry;
		for(int32 j = 0; j < g->numMaterials; j++){
			Pipeline *mp = g->materialList[j]->pipeline;
			if(ap && mp)
				printf("%s %x %x\n", argv[arg], ap->pluginData, mp->pluginData);
		}
	}
*/

	for(int32 i = 0; i < c->numAtomics; i++){
		Atomic *a = c->atomicList[i];
		ObjPipeline *p = a->getPipeline();
		if(uninstance)
			p->uninstance(a);
		else
			p->instance(a);
	}

	if(uninstance)
		rw::platform = rw::PLATFORM_D3D8;

	data = new rw::uint8[1024*1024];
	rw::StreamMemory out;
	out.open(data, 0, 1024*1024);
	if(currentUVAnimDictionary)
		currentUVAnimDictionary->streamWrite(&out);
	c->streamWrite(&out);

	FILE *cf;
	if(arg+1 < argc)
		cf = fopen(argv[arg+1], "wb");
	else
		cf = fopen("out.dff", "wb");
	assert(cf != NULL);
	fwrite(data, out.getLength(), 1, cf);
	fclose(cf);
	out.close();
	delete[] data;

	delete c;

	return 0;
}