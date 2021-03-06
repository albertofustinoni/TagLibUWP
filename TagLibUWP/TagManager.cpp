#include "TagManager.h"

#include "FileStreamRT.h"
#include "fileref.h"
#include "mpeg\mpegfile.h"

using namespace Windows::Storage;

namespace TagLibUWP
{
	AudioFileInfo^ TagManager::ReadFile(IStorageFile^ file)
	{
		TagLib::FileStreamRT fileStream(file, FileAccessMode::Read);
		TagLib::FileRef tagLibfile(&fileStream);
		ValidateFile(tagLibfile);

		auto readTag = ref new Tag(*tagLibfile.tag());
		auto readProperties = ref new AudioProperties(*tagLibfile.audioProperties());
		auto output = ref new AudioFileInfo(file, readTag, readProperties);
		return output;
	}

	void TagManager::WriteFile(AudioFileInfo^ fileinfo)
	{
		WriteFile(fileinfo->File, fileinfo->Tag);
	}

	void TagManager::WriteFile(IStorageFile^ file, Tag^ tag)
	{
		TagLib::FileStreamRT fileStream(file, FileAccessMode::ReadWrite);
		TagLib::FileRef tagLibfile(&fileStream);
		ValidateFile(tagLibfile);

		tag->UpdateTag(*tagLibfile.tag());

		//Ensure mp3 files are saved with id3v2 tags only
		auto wrappedFile = tagLibfile.file();
		auto fileAsMp3 = dynamic_cast<TagLib::MPEG::File*>(wrappedFile);
		if (fileAsMp3 != nullptr)
		{
			fileAsMp3->save(TagLib::MPEG::File::ID3v2, true, 3, false);
			return;
		}

		tagLibfile.save();
	}

	Array<String^>^ const TagManager::supportedExtensions = TagManager::GetSupportedExtensions();

	Array<String^>^ TagManager::GetSupportedExtensions()
	{
		auto supportedExts = TagLib::FileRef::defaultFileExtensions();
		auto output = ref new Array<String^>(supportedExts.size());
		for (auto i = 0U; i < output->Length; i++)
		{
			output[i] = ref new String(supportedExts[i].toCWString());
		}
		return output;
	}

	void TagManager::ValidateFile(const TagLib::FileRef& file)
	{
		if (file.isNull())
		{
			throw ref new FailureException(UnsupportedFileFormatErrorMsg);
		}
		if (!file.file()->isValid())
		{
			throw ref new FailureException(InvalidFileErrorMsg);
		}
	}
}