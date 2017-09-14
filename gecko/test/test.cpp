#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string>

#include "AudioStream.h"
#include "nsString.h"
#include "nsTArray.h"
#include "mozilla/ArrayUtils.h"
#include "nsDataHashtable.h"
#include "nsClassHashtable.h"
#include "nsRefPtrHashtable.h"
// #include "nsThreadUtils.h"

#define SIMPLE_STRING "I'm a simple ASCII string"
#define UTF8_STRING "Андропов, Брежнев, Горбачёв, Ленин, Маленков, Сталин, Хрущёв, Черненко"

namespace mozilla {

void TestString() {
  std::string std_utf8 = SIMPLE_STRING;
  std::u16string std_utf16 = u"" SIMPLE_STRING;

  nsCString moz_utf8(NS_LITERAL_CSTRING(SIMPLE_STRING));
  assert(std_utf8 == moz_utf8.get());
  assert(std_utf8.size() == moz_utf8.Length());

  nsString moz_utf16(NS_LITERAL_STRING(SIMPLE_STRING));
  assert(std_utf16 == moz_utf16.get());

  std_utf16 = u"" UTF8_STRING;
  moz_utf16 = NS_ConvertUTF8toUTF16(NS_LITERAL_CSTRING(UTF8_STRING));
  assert(std_utf16 == moz_utf16.get());
}

void TestArray() {
  nsTArray<uint32_t> a;
  uint32_t data[] = {0,1,2,3,4,5,6,7,8,9,10};
  a.AppendElements(data, ArrayLength(data));
  assert(memcmp(a.Elements(), data, ArrayLength(data) * sizeof(data[0])) == 0);
  auto i = 0;
  for (auto value : a) {
    assert(data[i++] == value);
  }
}

  void TestAudioStream() {
    class FakeChunk : public AudioStream::Chunk {
    public:
      FakeChunk() {}
      const AudioDataValue* Data() const { return NULL; }
      uint32_t Frames() const { return 0; }
      uint32_t Channels() const { return 2; }
      uint32_t Rate() const { return 44100; }
      AudioDataValue* GetWritable() const { return NULL; }
    };
    class FakeAudioSource : public AudioStream::DataSource {
    public:
      FakeAudioSource() {}
      UniquePtr<AudioStream::Chunk> PopFrames(uint32_t aFrames) {
        return MakeUnique<FakeChunk>();
      };
      bool Ended() const { return false; };
      void Drained() {};
    };

    auto* dataSource = new FakeAudioSource();
    auto* audioStream = new AudioStream(*dataSource);
    // FIXME: This doesn't work because CubebUtils doesn't build yet.
    // auto result = audioStream->Init(2, 1, 44100);
    // printf("result: %d\n", result);
    printf("audioStream: %p\n", audioStream);
  }
// void TestThreads() {
//   RefPtr<nsIThread> thread;]]
//   nsresult rv = NS_NewThread(getter_AddRefs(thread));
//   int x = 0;
//   RefPtr<Runnable> task(NS_NewRunnableFunction("TestFunction", [&]() {
//     x = 1;
//   }));
//   assert(NS_SUCCEEDED(rv));
//   assert(thread != nullptr);
//   thread->Dispatch(task.forget());
//   thread->Shutdown();
//   assert(x == 1);
// }

// TODO: Remove this once the real implementation in nsThreadManager.cpp lands.
PRThread* GetCurrentVirtualThread() { return nullptr; }

void TestHashTables() {
  static const char* values[] = {"zero", "one", "two", "three", "four"};
  {
    nsDataHashtable<nsUint32HashKey, const char*> table;
    for (uint32_t i = 0; i < ArrayLength(values); i++) {
      table.Put(i, values[i]);
    }
    for (uint32_t i = 0; i < ArrayLength(values); i++) {
      assert(table.Get(i) == values[i]);
    }
  }
  {
    nsClassHashtable<nsPtrHashKey<const char>, nsCString> table;
    for (const char* value : values) {
      table.Put(value, new nsCString(value));
    }
    for (uint32_t i = 0; i < ArrayLength(values); i++) {
      assert(table.Get(values[i])->EqualsASCII(values[i]));
    }

    for (auto iter = table.Iter(); !iter.Done(); iter.Next()) {
      const char* key = iter.Key();
      const nsCString* data = iter.UserData();
      assert(data->EqualsASCII(key));
      assert(key != nullptr);
    }
  }
  {
    class Data {
    public:
      NS_INLINE_DECL_REFCOUNTING(Data);
      uint32_t mIndex = -1;
    private:
      ~Data() {}
    };
    nsRefPtrHashtable<nsRefPtrHashKey<Data>, Data> table;
    nsTArray<RefPtr<Data>> datas;
    for (uint32_t i = 0; i < 5; i++) {
      RefPtr<Data> d = new Data();
      d->mIndex = i;
      datas.AppendElement(d);
      table.Put(d, d);
    }
    for (uint32_t i = 0; i < 5; i++) {
      RefPtr<Data> d;
      bool found = table.Get(datas[i], getter_AddRefs(d));
      assert(found);
      assert(d == datas[i]);
      assert(d->mIndex == i);
    }
  }
}

} // namespace mozilla

extern "C" void TestGecko() {
  mozilla::TestString();
  mozilla::TestArray();
  mozilla::TestAudioStream();
  mozilla::TestHashTables();
  // mozilla::TestThreads();
}
