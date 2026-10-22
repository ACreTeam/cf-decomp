#include "nw4r/snd/snd_WaveArchive.h"

/*******************************************************************************
 * headers
 */

#include <types.h>

#include "nw4r/snd/snd_Util.h"
#include "nw4r/snd/snd_WaveFile.h"

#include "nw4r/ut/ut_binaryFileFormat.h" // ut::BinaryFileHeader
#include "nw4r/ut/ut_algorithm.h" // ut::AddOffsetToPtr


/*******************************************************************************
 * functions
 */

namespace nw4r { namespace snd { namespace detail {

inline bool WaveArchiveReader::VerifyFileHeader(void const *waveArc)
{
	ut::BinaryFileHeader const *fileHeader =
		static_cast<ut::BinaryFileHeader const *>(waveArc);


	if (fileHeader->signature != WaveArchive::SIGNATURE_FILE)
		return false;

	// specifically not the source variant

	if (fileHeader->version < SUPPORTED_FILE_VERSION_MIN)
		return false;

	if (fileHeader->version > SUPPORTED_FILE_VERSION_MAX)
		return false;

	return true;
}

WaveArchiveReader::WaveArchiveReader(void const *waveArc) :
	mTableBlock	(nullptr),
	mDataBlock	(nullptr)
{

	if (!VerifyFileHeader(waveArc))
		return;

	WaveArchive::FileHeader const *fileHeader =
		static_cast<WaveArchive::FileHeader const *>(waveArc);

	WaveArchive::TableBlock const *tableBlock =
		static_cast<WaveArchive::TableBlock const *>(
			ut::AddOffsetToPtr(fileHeader, fileHeader->tableChunkOffset));


	WaveArchive::DataBlock const *dataBlock =
		static_cast<WaveArchive::DataBlock const *>(
			ut::AddOffsetToPtr(fileHeader, fileHeader->dataChunkOffset));


	mTableBlock	= tableBlock;
	mDataBlock	= dataBlock;
}

WaveFile::FileHeader const *WaveArchiveReader::GetWaveFile(int index) const
{
	if (!mTableBlock)
		return nullptr;

	if (!mDataBlock)
		return nullptr;

	if (index < 0)
		return nullptr;

	if (index >= mTableBlock->fileTable.count)
		return nullptr;

	Util::DataRef<WaveFile::FileHeader> waveFileRef =
		mTableBlock->fileTable.items[index].waveFileRef;

	WaveFile::FileHeader const *fileHeader =
		Util::GetDataRefAddress0(waveFileRef, mDataBlock);

	return fileHeader;
}


}}} // namespace nw4r::snd::detail
