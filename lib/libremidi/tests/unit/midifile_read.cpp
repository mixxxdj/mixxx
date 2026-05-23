#include "../include_catch.hpp"

#include <libremidi/reader.hpp>

#include <filesystem>

TEST_CASE("read valid files from corpus", "[midi_reader]")
{
  using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
  std::vector<uint8_t> bytes;
  constexpr const auto recursive = std::filesystem::directory_options::follow_directory_symlink;

  std::filesystem::path folder = LIBREMIDI_TEST_CORPUS;
  folder /= "Valid";

  const auto valid_midi_files = recursive_directory_iterator(folder, recursive);
  for (const auto& dirEntry : valid_midi_files)
  {
    INFO(dirEntry);

    if (dirEntry.is_regular_file())
    {
      if (dirEntry.path().extension() == ".mid")
      {
        bytes.clear();

        std::ifstream file{dirEntry.path(), std::ios::binary};

        bytes.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

        libremidi::reader r;
        libremidi::reader::parse_result result{};
        REQUIRE_NOTHROW(result = r.parse(bytes));
        CHECK(result == libremidi::reader::validated);
      }
    }
  }
}

TEST_CASE("read invalid files from corpus", "[midi_reader]")
{
  using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
  std::vector<uint8_t> bytes;
  constexpr const auto recursive = std::filesystem::directory_options::follow_directory_symlink;

  std::filesystem::path folder = LIBREMIDI_TEST_CORPUS;
  folder /= "Invalid";

  const auto valid_midi_files = recursive_directory_iterator(folder, recursive);
  for (const auto& dirEntry : valid_midi_files)
  {
    INFO(dirEntry);

    if (dirEntry.is_regular_file())
    {
      if (dirEntry.path().extension() == ".mid")
      {
        bytes.clear();

        std::ifstream file{dirEntry.path(), std::ios::binary};

        bytes.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

        libremidi::reader r;
        libremidi::reader::parse_result result{};
        REQUIRE_NOTHROW(result = r.parse(bytes));
        CHECK(result != libremidi::reader::validated);
      }
    }
  }
}

TEST_CASE("midi file duration in relative mode", "[midi_reader]")
{
  std::vector<uint8_t> bytes;
  std::ifstream file{
      LIBREMIDI_TEST_CORPUS "/Valid/MultiTrack/Middle/pilgrim.mid", std::ios::binary};
  bytes.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

  {
    libremidi::reader r{false};
    libremidi::reader::parse_result result{};
    REQUIRE_NOTHROW(result = r.parse(bytes));
    CHECK(result == libremidi::reader::validated);
    REQUIRE(r.get_end_time() == 75388.);
  }
}

TEST_CASE("midi file duration in absolute mode", "[midi_reader]")
{
  std::vector<uint8_t> bytes;
  std::ifstream file{
      LIBREMIDI_TEST_CORPUS "/Valid/MultiTrack/Middle/pilgrim.mid", std::ios::binary};
  bytes.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

  {
    libremidi::reader r{true};
    libremidi::reader::parse_result result{};
    REQUIRE_NOTHROW(result = r.parse(bytes));
    CHECK(result == libremidi::reader::validated);
    REQUIRE(r.get_end_time() == 75388.);
  }
}
