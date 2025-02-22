# remotes::install_github("r-dbi/dblog")
# Then, use dblog::dblog(duckdb::duckdb()) in conjunction with DBItest::test_some()
# to see the DBI calls emitted by the tests
DBItest::make_context(
  duckdb::duckdb(),
  # dblog::dblog(duckdb::duckdb()),
  list(debug = F),
  tweaks = DBItest::tweaks(
    omit_blob_tests = TRUE,
    temporary_tables = FALSE,
    placeholder_pattern = "?",
    timestamp_cast = function(x) sprintf("CAST('%s' AS TIMESTAMP)", x),
    date_cast = function(x) sprintf("CAST('%s' AS DATE)", x),
    time_cast = function(x) sprintf("CAST('%s' AS TIME)", x)
  ),
  name = "duckdb"
)
